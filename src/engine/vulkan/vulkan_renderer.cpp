#include "vulkan_renderer.h"

#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "core/entity.h"
#include "core/resource_handles.h"
#include "core/scene.h"
#include "engine/resource_loader.h"
#include "engine/resource_manager.h"
#include "engine/ubo.h"
#include "engine/vulkan/vulkan_depth_buffer.h"
#include "engine/vulkan/vulkan_descriptor_pool.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_gpu_buffer.h"
#include "engine/vulkan/vulkan_pipeline_manager.h"
#include "engine/vulkan/vulkan_render_utils.h"
#include "graphics/camera.h"
#include "graphics/color.h"
#include "graphics/model.h"
#include "graphics/renderable.h"
#include "imgui/imgui_wrapper.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

using namespace std::literals;

class VulkanSurface;

// TODO update to use Sync2 objects
VulkanRenderer::VulkanRenderer(
    const VulkanDevice &device,
    const VulkanSurface &surface,
    const ResourceManager &resource_manager,
    VulkanPipelineManager &pipeline_manager,
    VulkanDescriptorPool &descriptor_pool,
    std::uint32_t max_frames_in_flight,
    const Color clear_color)
    : max_frames_in_flight_{max_frames_in_flight}
    , device_{device}
    , resource_manager_{resource_manager}
    , pipeline_manager_{pipeline_manager}
    , descriptor_pool_{descriptor_pool}
    , swapchain_{device_, surface}
    , view_proj_uniform_buffers_(
          [&]() -> std::vector<VulkanGpuBuffer>
          {
              auto buffers = std::vector<VulkanGpuBuffer>();
              for (std::size_t i = 0; i < max_frames_in_flight; ++i)
              {
                  buffers.push_back(
                      {device_,
                       ::vk::DeviceSize{sizeof(UBO_ViewProj)},
                       ::vk::BufferUsageFlagBits::eUniformBuffer,
                       ::vk::MemoryPropertyFlagBits::eHostCoherent | ::vk::MemoryPropertyFlagBits::eHostVisible});
              }
              return buffers;
          }())
    , frame_command_context_{device_, max_frames_in_flight_}
    , depth_buffer_{device_, swapchain_.extent()}
    , per_frame_descriptor_sets_{}
    , clear_color_{clear_color.r, clear_color.g, clear_color.b, clear_color.a}
{
    arm::log::debug("VulkanRenderer constructor");
    per_frame_descriptor_sets_ = descriptor_pool_.allocate_per_frame_descriptor_sets(
        pipeline_manager_.get_per_frame_descriptor_set_layout(), view_proj_uniform_buffers_);
}

// returns true if swapchain is recreated, false if minimized
auto VulkanRenderer::recreate_resources() -> void
{
    swapchain_.recreate();
    depth_buffer_ = {device_, swapchain_.extent()};
    needs_recreate_ = false;
}

auto VulkanRenderer::needs_recreate() const -> bool
{
    return needs_recreate_;
}

auto VulkanRenderer::swapchain_image_count() const -> std::uint32_t
{
    return swapchain_.image_count();
}

auto VulkanRenderer::swapchain_image_index() const -> std::uint32_t
{
    return current_swap_chain_image_index_;
}

auto VulkanRenderer::swapchain_format() const -> ::vk::Format
{
    return swapchain_.format();
}

auto VulkanRenderer::descriptor_pool() -> VulkanDescriptorPool *
{
    return &descriptor_pool_;
}

auto VulkanRenderer::set_clear_color(const Color &color) -> void
{
    clear_color_ = {color.r, color.g, color.b, color.a};
}

auto VulkanRenderer::render(const Scene &scene, const Camera &camera, ImDrawData *imgui_draw_data) -> void
{
    auto render_status = prepare_frame_(scene);

    switch (render_status.code)
    {
        case RenderStatusCode::ReadyToRecord:
        {
            record_(render_status.draw_items, camera, imgui_draw_data);
            end_frame_();
        }
        break;

        case RenderStatusCode::SkipMinimized:
        case RenderStatusCode::RecreateRequested:
        {
            return;
        }

        case RenderStatusCode::Error:
        default:
        {
            throw arm::Exception("render error");
        }
    }
}

auto VulkanRenderer::shutdown() -> void
{
    per_frame_descriptor_sets_.clear();
}

auto VulkanRenderer::prepare_frame_(const Scene &scene) -> RenderStatus
{
    // recreates the swapchain if out of date, otherwise gets an image from the swapchain
    frame_command_context_.wait_for_fence();
    for (;;)
    {
        // TODO review this
        // bypassing raii here because Vulkan internally checks Result and asserts on OutOfDate error at acquire.
        // we're handling OutOfDate errors, so we'll use the C API for this (and also for present) to avoid the internal
        // check.
        auto swapchain = static_cast<VkSwapchainKHR>(*swapchain_.native_handle());
        auto semaphore = static_cast<VkSemaphore>(*frame_command_context_.current_image_available_semaphore());

        auto next_image_info = VkAcquireNextImageInfoKHR{};
        next_image_info.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
        next_image_info.pNext = NULL;
        next_image_info.swapchain = swapchain;
        next_image_info.timeout = UINT64_MAX;
        next_image_info.semaphore = semaphore;
        next_image_info.fence = NULL;
        next_image_info.deviceMask = 1u;

        auto swap_chain_image_index = std::uint32_t{};
        auto vk_result = vkAcquireNextImage2KHR(*device_.native_handle(), &next_image_info, &swap_chain_image_index);

        if (vk_result == VK_SUCCESS || vk_result == VK_SUBOPTIMAL_KHR)
        {
            current_swap_chain_image_index_ = swap_chain_image_index;
            needs_recreate_ |= (vk_result == VK_SUBOPTIMAL_KHR);
            break;
        }
        if (vk_result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            return {RenderStatusCode::RecreateRequested, {}};
        }
        throw arm::Exception(
            "Unable to aquire swapchain image ({})", ::vk::to_string(static_cast<::vk::Result>(vk_result)));
    }

    // we have a swapchain image at this point, though swapchain could be suboptimal. render anyway. this may change

    // flatten entities -> draw_items, then sort by sort_key (tuple of pipeline_id, material_handle, mesh_handle,
    // depth_bucket)

    auto visit = [&](this auto &&self,
                     EntityIndex entity_index,
                     ::glm::mat4 parent_transform,
                     std::vector<DrawItem> &draw_items) -> void
    {
        const auto &entity = scene.entities().at(entity_index.value);
        const auto world_transform = parent_transform * ::glm::mat4(entity.transform());

        if (entity.model().has_value())
        {
            for (const auto &renderable : entity.model().value().renderables)
            {
                auto draw_item = DrawItem{};
                draw_item.mesh_handle = renderable.mesh_handle;
                draw_item.material_handle = renderable.material_handle.has_value()
                                                ? std::make_optional<MaterialHandle>(renderable.material_handle.value())
                                                : std::nullopt;
                draw_item.model = world_transform;
                // TODO: this can't always get default pipeline key
                draw_item.sort_key = make_draw_sort_key_(
                    pipeline_manager_.get_default_pipeline_key(), draw_item.material_handle, draw_item.mesh_handle);
                draw_items.push_back(std::move(draw_item));
            }
        }

        if (entity.child_count() > 0zu)
        {
            for (const auto &index : entity.children())
            {
                self(index, world_transform, draw_items);
            }
        }
    };

    // TODO this will need a complete refactor once new pipeline is in place
    auto result = std::vector<DrawItem>();
    for (const auto &entity_index : scene.root_indices())
    {
        visit(entity_index, ::glm::mat4x4(1.0f), result);
    }

    std::ranges::sort(result, {}, &DrawItem::sort_key);
    return {RenderStatusCode::ReadyToRecord, std::move(result)};
}

auto VulkanRenderer::record_(const std::vector<DrawItem> &draw_items, const Camera &camera, ImDrawData *imgui_draw_data)
    -> void
{
    const auto frame_index = frame_command_context_.current_frame_index();

    // prepare infos
    auto color_attachment_info =
        make_color_attachment(*swapchain_.image_views()[current_swap_chain_image_index_], clear_color_);
    auto depth_attachment_info = make_depth_attachment(depth_buffer_.image_view());
    auto rendering_info = ::vk::RenderingInfo{};
    rendering_info.sType = ::vk::StructureType::eRenderingInfo;
    rendering_info.pNext = nullptr;
    rendering_info.flags = {};
    rendering_info.renderArea.offset = ::vk::Offset2D{0, 0};
    rendering_info.renderArea.extent = swapchain_.extent();
    rendering_info.layerCount = 1;
    rendering_info.viewMask = 0; // TODO magic-y number - must match pipeline creation create_rendering_info
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment_info;
    rendering_info.pDepthAttachment = &depth_attachment_info;
    rendering_info.pStencilAttachment = nullptr;

    // update view/projection matrix data into UBO
    auto temp_view_proj = UBO_ViewProj{
        .view = camera.get_view_matrix(),
        .proj = ::glm::perspective(
            ::glm::radians(45.0f),
            static_cast<float>(swapchain_.extent().width) / swapchain_.extent().height,
            0.1f,
            100.0f)};
    temp_view_proj.proj[1][1] *= -1.0f;
    view_proj_uniform_buffers_[frame_index].upload(&temp_view_proj, sizeof(UBO_ViewProj));

    // start command buffer
    auto &command_buffer = frame_command_context_.current_command_buffer();
    const auto command_buffer_begin_info =
        ::vk::CommandBufferBeginInfo{::vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    command_buffer.reset();
    command_buffer.begin(command_buffer_begin_info);

    // transition images for recording
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

    // render
    command_buffer.beginRendering(rendering_info);

    command_buffer.bindPipeline(
        ::vk::PipelineBindPoint::eGraphics,
        *pipeline_manager_.get_pipeline(pipeline_manager_.get_default_pipeline_key()).pipeline);
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
        ::vk::PipelineBindPoint::eGraphics,
        *pipeline_manager_.get_pipeline_layout(),
        0,
        *per_frame_descriptor_sets_.at(frame_index),
        nullptr);

    //    iterate draw items
    auto last_mesh = static_cast<const Mesh *>(nullptr);
    auto last_material = static_cast<const Material *>(nullptr);
    for (const auto &draw_item : draw_items)
    {
        // get mesh and update vertex/index buffers if the mesh has changed since the last draw
        auto &mesh = resource_manager_.get<Mesh>(draw_item.mesh_handle);
        if (&mesh != last_mesh)
        {
            last_mesh = &mesh;
            command_buffer.bindVertexBuffers(0, {mesh.vertex_buffer().native_handle()}, {0});
            command_buffer.bindIndexBuffer({mesh.index_buffer().native_handle()}, 0, ::vk::IndexType::eUint32);
        }

        // update model matrix
        command_buffer.pushConstants<::glm::mat4>(
            *pipeline_manager_.get_pipeline_layout(), ::vk::ShaderStageFlagBits::eVertex, 0u, draw_item.model);

        // update material data
        if (draw_item.material_handle.has_value())
        {
            auto &draw_item_material = resource_manager_.get<Material>(draw_item.material_handle.value());
            if (&draw_item_material != last_material)
            {
                last_material = &draw_item_material;
                command_buffer.bindDescriptorSets(
                    ::vk::PipelineBindPoint::eGraphics,
                    *pipeline_manager_.get_pipeline_layout(),
                    1,
                    *draw_item_material.descriptor_set(),
                    nullptr);
            }
        }
        // draw the current entity
        command_buffer.drawIndexed(mesh.index_count(), 1, 0, 0, 0);
    }

    // TODO debug guards?
    // draw imgui if it has things for us to draw
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

    frame_command_context_.reset_fence();
    auto submit_result = device_.graphics_queue().submit(submit_info, *frame_command_context_.current_fence());
    if (submit_result != ::vk::Result::eSuccess)
    {
        // TODO handle this more gracefully
        throw arm::Exception("graphics queue submit failed");
    }

    // TODO review this
    // bypassing raii here because Vulkan internally checks Result and asserts on OutOfDate error at present.
    // we're handling OutOfDate errors, so we'll use the C API for this (and also for acquire) to avoid the internal
    // check.
    auto semaphore = static_cast<VkSemaphore>(*swapchain_.semaphores().at(current_swap_chain_image_index_));
    auto swapchain = static_cast<VkSwapchainKHR>(*swapchain_.native_handle());
    auto present_info = VkPresentInfoKHR{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain;
    present_info.pImageIndices = &current_swap_chain_image_index_;

    const auto present_result = vkQueuePresentKHR(device_.graphics_queue(), &present_info);
    if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR)
    {
        needs_recreate_ = true;
        if (present_result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            frame_command_context_.advance_frame();
            return;
        }
    }

    frame_command_context_.advance_frame();
    ++frame_counter_;
}

constexpr auto VulkanRenderer::make_draw_sort_key_(
    PipelineKey pipeline_id,
    std::optional<MaterialHandle> material_handle,
    MeshHandle mesh_handle,
    std::int32_t depth_bucket) -> DrawSortKey
{
    constexpr auto no_material = MaterialHandle{std::numeric_limits<std::uint64_t>::max() - 1zu};
    if (material_handle.has_value())
    {
        return {pipeline_id.pack(), material_handle.value(), mesh_handle, depth_bucket};
    }
    return {pipeline_id.pack(), no_material, mesh_handle, depth_bucket};
}

} // namespace pong
