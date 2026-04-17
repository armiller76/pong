#include "vulkan_renderer.h"

#include <array>
#include <cstddef>
#include <ranges>
#include <tuple>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "core/entity.h"
#include "core/resource_handles.h"
#include "engine/resource_manager.h"
#include "engine/ubo.h"
#include "graphics/camera.h"
#include "graphics/color.h"
#include "graphics/model.h"
#include "imgui/imgui_wrapper.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/log.h"
#include "vulkan_depth_buffer.h"
#include "vulkan_device.h"
#include "vulkan_render_utils.h"

namespace pong
{

// TODO update to use Sync2 objects
VulkanRenderer::VulkanRenderer(
    const VulkanDevice &device,
    const VulkanSurface &surface,
    const Camera &camera,
    ResourceManager &resource_manager,
    std::uint32_t max_frames_in_flight,
    const Color clear_color)
    : max_frames_in_flight_{max_frames_in_flight}
    , device_{device}
    , surface_{surface}
    , camera_{camera}
    , resource_manager_{resource_manager}
    , swapchain_{device_, surface_}
    , frame_command_context_{device_, swapchain_.image_count(), max_frames_in_flight_}
    , uniform_buffers_(
          [&device, max_frames_in_flight]() -> std::vector<VulkanGpuBuffer>
          {
              auto buffers = std::vector<VulkanGpuBuffer>();
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
    , depth_buffer_{device_, swapchain_.extent()}
    , descriptor_pool_{device_, uniform_buffers_, max_frames_in_flight_}
    , pipeline_factory_{device_, descriptor_pool_, resource_manager_}
    , pipeline_resources_{pipeline_factory_.create_graphics_pipeline(swapchain_.format(), depth_buffer_.format())}
    , descriptor_sets_{descriptor_pool_.allocate_descriptor_sets(
          pipeline_resources_.descriptor_set_layouts.at(0),
          max_frames_in_flight_)}
    , render_order_{}
    , clear_color_{clear_color.r, clear_color.g, clear_color.b, clear_color.a}
{
    arm::log::debug("VulkanRenderer constructor");
}

auto VulkanRenderer::recreate_resources() -> void
{
    // TODO eventually, let's not waitidle unless absolutely needed (ie don't call this every frame of a resize)
    device_.native_handle().waitIdle();
    swapchain_.recreate();
    depth_buffer_ = {device_, swapchain_.extent()};
    if (imgui_resize_callback)
    {
        imgui_resize_callback();
    }
    framebuffer_resized_ = false;
}

auto VulkanRenderer::set_clear_color(const Color &color) -> void
{
    clear_color_ = {color.r, color.g, color.b, color.a};
}

auto VulkanRenderer::render(const std::vector<Entity> &entities, ImDrawData *imgui_draw_data) -> void
{
    prepare_frame_();
    record_(entities, imgui_draw_data);
    end_frame_();

    if (framebuffer_resized_)
    {
        recreate_resources();
    }
}

auto VulkanRenderer::prepare_frame_() -> void
{
    frame_command_context_.wait_current_frame();

    for (;;)
    {
        auto [result, swap_chain_image_index] = swapchain_.native_handle().acquireNextImage(
            UINT64_MAX, frame_command_context_.current_image_available_semaphore(), VK_NULL_HANDLE);

        using enum ::vk::Result;
        if (result == eSuccess || result == eSuboptimalKHR)
        {
            current_swap_chain_image_index_ = swap_chain_image_index;
            framebuffer_resized_ |= (result == eSuboptimalKHR);
            break;
        }
        if (result == eErrorOutOfDateKHR)
        {
            recreate_resources();
            continue;
        }
        throw arm::Exception("Unable to aquire swapchain image ({})", ::vk::to_string(result));
    }
    auto &command_buffer = frame_command_context_.current_command_buffer();

    const auto command_buffer_begin_info =
        ::vk::CommandBufferBeginInfo{::vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    command_buffer.reset();
    command_buffer.begin(command_buffer_begin_info);

    transition(
        command_buffer,
        swapchain_.images().at(current_swap_chain_image_index_),
        ::vk::ImageAspectFlagBits::eColor,
        transition_info::undef_to_color_optimal());
    transition(
        command_buffer,
        depth_buffer_.image(),
        ::vk::ImageAspectFlagBits::eDepth | ::vk::ImageAspectFlagBits::eStencil,
        transition_info::undef_to_depth_optimal());
}

auto VulkanRenderer::record_(const std::vector<Entity> &entities, ImDrawData *imgui_draw_data) -> void
{
    const auto frame_index = frame_command_context_.current_frame_index();
    auto &command_buffer = frame_command_context_.current_command_buffer();

    auto color_attachment_info = ::vk::RenderingAttachmentInfo{};
    color_attachment_info.sType = ::vk::StructureType::eRenderingAttachmentInfo;
    color_attachment_info.pNext = nullptr;
    color_attachment_info.imageView = swapchain_.image_views().at(current_swap_chain_image_index_);
    color_attachment_info.imageLayout = transition_info::undef_to_color_optimal().dst_layout;
    color_attachment_info.loadOp = ::vk::AttachmentLoadOp::eClear;
    color_attachment_info.storeOp = ::vk::AttachmentStoreOp::eStore;
    color_attachment_info.clearValue = ::vk::ClearColorValue(clear_color_);
    color_attachment_info.resolveMode = ::vk::ResolveModeFlagBits::eNone;
    color_attachment_info.resolveImageView = nullptr;
    color_attachment_info.resolveImageLayout = ::vk::ImageLayout::eUndefined;

    auto depth_attachment_info = ::vk::RenderingAttachmentInfo{};
    depth_attachment_info.sType = ::vk::StructureType::eRenderingAttachmentInfo;
    depth_attachment_info.pNext = nullptr;
    depth_attachment_info.imageView = depth_buffer_.image_view();
    depth_attachment_info.imageLayout = transition_info::undef_to_depth_optimal().dst_layout;
    depth_attachment_info.loadOp = ::vk::AttachmentLoadOp::eClear;
    depth_attachment_info.storeOp = ::vk::AttachmentStoreOp::eDontCare;
    depth_attachment_info.clearValue = ::vk::ClearDepthStencilValue{1.0f, 0};
    depth_attachment_info.resolveMode = ::vk::ResolveModeFlagBits::eNone;
    depth_attachment_info.resolveImageView = nullptr;
    depth_attachment_info.resolveImageLayout = ::vk::ImageLayout::eUndefined;

    auto rendering_info = ::vk::RenderingInfo{};
    rendering_info.sType = ::vk::StructureType::eRenderingInfo;
    rendering_info.pNext = nullptr;
    rendering_info.flags = {};
    rendering_info.renderArea.offset = ::vk::Offset2D{0, 0};
    rendering_info.renderArea.extent = swapchain_.extent();
    rendering_info.layerCount = 1;
    rendering_info.viewMask = 0;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment_info;
    rendering_info.pDepthAttachment = &depth_attachment_info;
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

    // update view/projection matrix data into UBO
    auto temp_view_proj = ubo_vp{
        .view = camera_.get_view_matrix(),
        .proj = ::glm::perspective(
            ::glm::radians(45.0f),
            static_cast<float>(swapchain_.extent().width) / swapchain_.extent().height,
            0.1f,
            100.0f)};
    uniform_buffers_[frame_index].upload(&temp_view_proj, sizeof(temp_view_proj));

    render_order_ = std::views::iota(std::size_t{0}, entities.size()) | std::ranges::to<std::vector<std::size_t>>();
    std::ranges::sort(
        render_order_,
        {},
        [&](std::size_t i)
        {
            constexpr auto pipeline_id = std::uint64_t{0}; // for future use
            const auto material_id = entities[i].model().renderables[0].material_handle.has_value()
                                         ? entities[i].model().renderables[0].material_handle.value().value
                                         : 0zu;
            const auto mesh_id = entities[i].model().renderables[0].mesh_handle.value;
            return make_draw_sort_key(pipeline_id, material_id, mesh_id);
        });

    auto last_mesh = static_cast<const Mesh *>(nullptr);
    for (auto &i : render_order_)
    {
        // get mesh and update vertex/index buffers if it changed since last draw
        auto &mesh = resource_manager_.get<Mesh>(entities[i].model().renderables[0].mesh_handle);
        if (&mesh != last_mesh)
        {
            last_mesh = &mesh;
            command_buffer.bindVertexBuffers(0, {mesh.vertex_buffer().native_handle()}, {0});
            command_buffer.bindIndexBuffer({mesh.index_buffer().native_handle()}, 0, ::vk::IndexType::eUint32);
        }

        // update model matrix. note that pong::Transform is overloaded to implicitly convert to a mat4
        const auto model = ::glm::mat4(entities[i].transform());
        command_buffer.pushConstants<::glm::mat4>(
            *pipeline_resources_.layout, ::vk::ShaderStageFlagBits::eVertex, 0u, model);

        // draw the current entity
        command_buffer.drawIndexed(mesh.index_count(), 1, 0, 0, 0);
    }

    if (imgui_draw_data != nullptr)
    {
        ImGui_ImplVulkan_RenderDrawData(imgui_draw_data, *command_buffer);
    }

    command_buffer.endRendering();
}

auto VulkanRenderer::end_frame_() -> void
{
    auto &command_buffer = frame_command_context_.current_command_buffer();
    transition(
        command_buffer,
        swapchain_.images().at(current_swap_chain_image_index_),
        ::vk::ImageAspectFlagBits::eColor,
        transition_info::color_optimal_to_present());
    command_buffer.end();

    auto wait_for = ::vk::PipelineStageFlags{::vk::PipelineStageFlagBits::eColorAttachmentOutput};
    auto submit_info = ::vk::SubmitInfo{};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &*frame_command_context_.current_image_available_semaphore();
    submit_info.pWaitDstStageMask = &wait_for;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &*frame_command_context_.current_command_buffer();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &*swapchain_.semaphores().at(current_swap_chain_image_index_);

    device_.graphics_queue().submit(submit_info, *frame_command_context_.current_fence());

    auto present_info = ::vk::PresentInfoKHR{};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &*swapchain_.semaphores().at(current_swap_chain_image_index_);
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &*swapchain_.native_handle();
    present_info.pImageIndices = &current_swap_chain_image_index_;

    const auto present_result = device_.present_queue().presentKHR(present_info);
    if (present_result == ::vk::Result::eErrorOutOfDateKHR || present_result == ::vk::Result::eSuboptimalKHR)
    {
        framebuffer_resized_ = true;
    }

    frame_command_context_.advance_frame();
}

} // namespace pong
