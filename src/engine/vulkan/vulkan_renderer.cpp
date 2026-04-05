#include "vulkan_renderer.h"

#include <array>
#include <cstddef>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "core/entity.h"
#include "engine/resource_manager.h"
#include "engine/ubo.h"
#include "graphics/color.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/log.h"
#include "vulkan_device.h"
#include "vulkan_layout_transition.h"

namespace pong
{

VulkanRenderer::VulkanRenderer(
    const VulkanDevice &device,
    const VulkanSurface &surface,
    ResourceManager &resource_manager,
    std::uint32_t max_frames_in_flight,
    const Color clear_color)
    : max_frames_in_flight_{max_frames_in_flight}
    , device_{device}
    , surface_{surface}
    , resource_manager_{resource_manager}
    , swapchain_{device_, surface_}
    , command_context_{device_, swapchain_.image_count(), max_frames_in_flight_}
    , uniform_buffers_(
          [&device, max_frames_in_flight]() -> std::vector<GpuBuffer>
          {
              auto buffers = std::vector<GpuBuffer>();
              for (std::size_t i = 0; i < max_frames_in_flight; ++i)
              {
                  buffers.push_back(
                      {device,
                       ::vk::DeviceSize{sizeof(ubo_vp)},
                       ::vk::BufferUsageFlagBits::eUniformBuffer,
                       ::vk::MemoryPropertyFlagBits::eHostCoherent | ::vk::MemoryPropertyFlagBits::eHostVisible});
              }
              return buffers;
          }())
    , descriptor_pool_{device_, uniform_buffers_, max_frames_in_flight_}
    , pipeline_factory_{device_, descriptor_pool_, resource_manager_}
    , pipeline_resources_{pipeline_factory_.create_graphics_pipeline(
          resource_manager_.get_resource_id("simple.vert"),
          resource_manager_.get_resource_id("simple.frag"),
          swapchain_.format())}
    , descriptor_sets_{descriptor_pool_.allocate_descriptor_sets(
          pipeline_resources_.descriptor_set_layouts.at(0),
          max_frames_in_flight_)}
    , render_order_{}
    , clear_color_{clear_color.r, clear_color.g, clear_color.b, clear_color.a}
{
    arm::log::debug("VulkanRenderer constructor");
}

auto VulkanRenderer::framebuffer_resized() -> void
{
    // TODO anything else?
    swapchain_.recreate();
}

auto VulkanRenderer::set_clear_color(const Color &color) -> void
{
    // TODO: GIGO
    clear_color_ = {color.r, color.g, color.b, color.a};
}

auto VulkanRenderer::render(const std::vector<Entity> &entities /*Scene &scene, Camera &camera, etc*/) -> void
{
    prepare_frame_();
    record_(entities /*scene, camera, etc*/);
    end_frame_();
}

auto VulkanRenderer::prepare_frame_() -> void
{
    command_context_.wait_current_frame();

    for (;;)
    {
        auto [result, swap_chain_image_index] = swapchain_.native_handle().acquireNextImage(
            UINT64_MAX, command_context_.current_image_available_semaphore(), VK_NULL_HANDLE);

        using enum ::vk::Result;
        if (result == eSuccess || result == eSuboptimalKHR)
        {
            current_swap_chain_image_index_ = swap_chain_image_index;
            framebuffer_resized_ |= (result == eSuboptimalKHR);
            break;
        }
        if (result == eErrorOutOfDateKHR)
        {
            swapchain_.recreate();
            framebuffer_resized_ = false;
            continue;
        }
        throw arm::Exception("Unable to aquire swapchain image ({})", ::vk::to_string(result));
    }
}

auto VulkanRenderer::record_(const std::vector<Entity> &entities /*pass in stuff to draw*/) -> void
{
    const auto frame_index = command_context_.current_frame_index();
    auto &command_buffer = command_context_.current_command_buffer();

    const auto command_buffer_begin_info =
        ::vk::CommandBufferBeginInfo{::vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    command_buffer.reset();
    command_buffer.begin(command_buffer_begin_info);

    transition_(current_swap_chain_image_index_, transition_info::undef_to_color_optimal());

    auto rendering_attachment_info = ::vk::RenderingAttachmentInfo{};
    rendering_attachment_info.sType = ::vk::StructureType::eRenderingAttachmentInfo;
    rendering_attachment_info.pNext = nullptr;
    rendering_attachment_info.imageView = swapchain_.image_views().at(current_swap_chain_image_index_);
    rendering_attachment_info.imageLayout = transition_info::undef_to_color_optimal().dst_layout;
    rendering_attachment_info.loadOp = ::vk::AttachmentLoadOp::eClear;
    rendering_attachment_info.storeOp = ::vk::AttachmentStoreOp::eStore;
    rendering_attachment_info.clearValue = clear_color_;
    rendering_attachment_info.resolveMode = ::vk::ResolveModeFlagBits::eNone;
    rendering_attachment_info.resolveImageView = nullptr;
    rendering_attachment_info.resolveImageLayout = ::vk::ImageLayout::eUndefined;

    auto rendering_info = ::vk::RenderingInfo{};
    rendering_info.sType = ::vk::StructureType::eRenderingInfo;
    rendering_info.pNext = nullptr;
    rendering_info.flags = {};
    rendering_info.renderArea.offset = ::vk::Offset2D{0, 0};
    rendering_info.renderArea.extent = swapchain_.extent();
    rendering_info.layerCount = 1;
    rendering_info.viewMask = 0;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &rendering_attachment_info;
    rendering_info.pDepthAttachment = nullptr;
    rendering_info.pStencilAttachment = nullptr;

    command_buffer.beginRendering(rendering_info);
    command_buffer.bindPipeline(::vk::PipelineBindPoint::eGraphics, pipeline_resources_.pipeline);

    command_buffer.setViewport(
        0,
        ::vk::Viewport{
            0.0f,
            0.0f,
            static_cast<float>(swapchain_.extent().width),
            static_cast<float>(swapchain_.extent().height),
            0.0f,
            1.0f});
    command_buffer.setScissor(0, ::vk::Rect2D{::vk::Offset2D{0, 0}, swapchain_.extent()});
    command_buffer.bindDescriptorSets(
        ::vk::PipelineBindPoint::eGraphics, pipeline_resources_.layout, 0, *descriptor_sets_.at(frame_index), nullptr);

    const auto last_mesh = static_cast<Mesh *>(nullptr);
    for (auto &entity : entities)
    {
        // get mesh and update vertex/index buffers if it changed since last draw
        auto &mesh = resource_manager_.get<Mesh>(entity.mesh_handle());
        if (&mesh != last_mesh)
        {
            command_buffer.bindVertexBuffers(0, {mesh.vertex_buffer().native_handle()}, {0});
            command_buffer.bindIndexBuffer({mesh.index_buffer().native_handle()}, 0, ::vk::IndexType::eUint32);
        }

        // update view/projection matrix data into UBO
        auto dummy_view_proj = ubo_vp{
            .view = ::glm::mat4(1.0f),
            .proj = ::glm::mat4(1.0f),
        };
        uniform_buffers_[frame_index].upload(&dummy_view_proj, sizeof(dummy_view_proj));

        // update model matrix. note that pong::Transform is overloaded to implicitly convert to a mat4
        const auto model = ::glm::mat4(entity.transform());
        command_buffer.pushConstants<::glm::mat4>(
            *pipeline_resources_.layout, ::vk::ShaderStageFlagBits::eVertex, 0u, model);

        // draw the current entity
        command_buffer.drawIndexed(mesh.index_count(), 1, 0, 0, 0);
    }
    command_buffer.endRendering();

    transition_(current_swap_chain_image_index_, transition_info::color_optimal_to_present());
    command_buffer.end();
}

auto VulkanRenderer::end_frame_() -> void
{
    auto wait_for = ::vk::PipelineStageFlags{::vk::PipelineStageFlagBits::eColorAttachmentOutput};
    auto submit_info = ::vk::SubmitInfo{};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &*command_context_.current_image_available_semaphore();
    submit_info.pWaitDstStageMask = &wait_for;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &*command_context_.current_command_buffer();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &*command_context_.render_finished_semaphore(current_swap_chain_image_index_);

    device_.graphics_queue().submit(submit_info, *command_context_.current_fence());

    auto present_info = ::vk::PresentInfoKHR{};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &*command_context_.render_finished_semaphore(current_swap_chain_image_index_);
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &*swapchain_.native_handle();
    present_info.pImageIndices = &current_swap_chain_image_index_;

    const auto present_result = device_.present_queue().presentKHR(present_info);
    if (present_result == ::vk::Result::eErrorOutOfDateKHR)
    {
        swapchain_.recreate();
    }

    command_context_.advance_frame();
}

auto VulkanRenderer::transition_(std::uint32_t swap_chain_image_index, transition_info info) -> void
{
    auto barrier = ::vk::ImageMemoryBarrier2{};
    barrier.sType = ::vk::StructureType::eImageMemoryBarrier2;
    barrier.srcStageMask = info.src_stage;
    barrier.srcAccessMask = info.src_access;
    barrier.dstStageMask = info.dst_stage;
    barrier.dstAccessMask = info.dst_access;
    barrier.oldLayout = info.src_layout;
    barrier.newLayout = info.dst_layout;
    // TODO when do we stop ignoring queue family index?
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // TODO is this safe? i think so if the compiler accepts it, but is it copying the image or moving or...what?
    barrier.image = swapchain_.images().at(swap_chain_image_index);

    barrier.subresourceRange.aspectMask = ::vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    auto dependency_info = ::vk::DependencyInfo{};
    dependency_info.dependencyFlags = {};
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &barrier;

    command_context_.current_command_buffer().pipelineBarrier2(dependency_info);
}

} // namespace pong
