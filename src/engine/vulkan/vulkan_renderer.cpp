#include "vulkan_renderer.h"

#include <cstdint>
#include <string_view>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "core/entity.h"
#include "core/resource_handles.h"
#include "core/scene.h"
#include "engine/resource_manager.h"
#include "engine/ubo.h"
#include "engine/vulkan/vulkan_depth_buffer.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_gpu_buffer.h"
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

// TODO update to use Sync2 objects
VulkanRenderer::VulkanRenderer(
    const VulkanDevice &device,
    const VulkanSurface &surface,
    std::uint32_t max_frames_in_flight,
    const Color clear_color)
    : max_frames_in_flight_{max_frames_in_flight}
    , device_{device}
    , surface_{surface}
    , resource_manager_{device_}
    , resource_loader_{device_, resource_manager_, "c:/dev/Pong/assets"sv}
    , swapchain_{device_, surface_}
    , frame_command_context_{device_, max_frames_in_flight_}
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
    , depth_buffer_{device_, swapchain_.extent()}
    , descriptor_pool_{device_, view_proj_uniform_buffers_, max_frames_in_flight_}
    , pipeline_factory_{device_, resource_manager_}
    , pipeline_resources_{pipeline_factory_.create_graphics_pipeline(swapchain_.format(), depth_buffer_.format())}
    , descriptor_sets_{descriptor_pool_.allocate_per_frame_descriptor_sets(
          pipeline_resources_.per_frame_descriptor_set_layout)}
    , clear_color_{clear_color.r, clear_color.g, clear_color.b, clear_color.a}
{
    arm::log::debug("VulkanRenderer constructor");
    resource_manager_.set_pipeline_resources(pipeline_resources_);
    resource_manager_.set_descriptor_pool(descriptor_pool_);
}

auto VulkanRenderer::shutdown() -> void
{
    device_.native_handle().waitIdle();
    descriptor_sets_.clear();
    resource_manager_.shutdown();
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

auto VulkanRenderer::load_scene(std::string_view filename) -> Scene
{
    return resource_loader_.loadgltf(filename);
}

auto VulkanRenderer::render(const Scene &scene, const Camera &camera, ImDrawData *imgui_draw_data) -> void
{
    auto draw_items = prepare_frame_(scene);
    record_(draw_items, camera, imgui_draw_data);
    end_frame_();

    if (framebuffer_resized_)
    {
        recreate_resources();
    }
}

auto VulkanRenderer::prepare_frame_(const Scene &scene) -> std::vector<DrawItem>
{
    // recreates the swapchain if out of date, otherwise gets an image from the swapchain
    frame_command_context_.wait_for_fence();
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
                draw_item.sort_key = make_draw_sort_key_(0zu, draw_item.material_handle.value(), draw_item.mesh_handle);
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

    auto result = std::vector<DrawItem>();
    for (const auto &entity_index : scene.root_indices())
    {
        visit(entity_index, ::glm::mat4x4(1.0f), result);
    }

    std::ranges::sort(result, {}, &DrawItem::sort_key);
    return result;
}

auto VulkanRenderer::record_(
    [[maybe_unused]] const std::vector<DrawItem> &draw_items,
    const Camera &camera,
    ImDrawData *imgui_draw_data) -> void
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
    rendering_info.viewMask = 0;
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
            *pipeline_resources_.layout, ::vk::ShaderStageFlagBits::eVertex, 0u, draw_item.model);

        // update material data
        if (draw_item.material_handle.has_value())
        {
            auto &draw_item_material = resource_manager_.get<Material>(draw_item.material_handle.value());
            if (&draw_item_material != last_material)
            {
                last_material = &draw_item_material;
                command_buffer.bindDescriptorSets(
                    ::vk::PipelineBindPoint::eGraphics,
                    pipeline_resources_.layout,
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

constexpr auto VulkanRenderer::make_draw_sort_key_(
    std::uint64_t pipeline_id,
    MaterialHandle material_handle,
    MeshHandle mesh_handle,
    std::int32_t depth_bucket) -> DrawSortKey
{
    return {pipeline_id, material_handle, mesh_handle, depth_bucket};
}

} // namespace pong
