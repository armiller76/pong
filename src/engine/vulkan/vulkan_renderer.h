#pragma once

#include <cstdint>
#include <functional>
#include <tuple>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "core/resource_handles.h"
#include "graphics/color.h"
#include "graphics/mesh.h"
#include "vulkan_depth_buffer.h"
#include "vulkan_descriptor_pool.h"
#include "vulkan_frame_command_context.h"
#include "vulkan_gpu_buffer.h"
#include "vulkan_pipeline_factory.h"
#include "vulkan_swapchain.h"

struct ImDrawData;

namespace pong
{

class ResourceManager;
class Scene;
class VulkanDevice;
class VulkanSurface;
class Camera;
struct Renderable;

class VulkanRenderer
{
    using DrawSortKey = std::tuple<std::uint64_t, MaterialHandle, MeshHandle, std::int32_t>;
    struct DrawItem
    {
        MeshHandle mesh_handle;
        std::optional<MaterialHandle> material_handle;
        ::glm::mat4 model;
        DrawSortKey sort_key;
    };

  public:
    VulkanRenderer(
        const VulkanDevice &device,
        const VulkanSurface &surface,
        ResourceManager &resource_manager,
        std::uint32_t max_frames_in_flight,
        const Color clear_color = {0.42f, 0.42f, 0.42f, 1.0f});
    ~VulkanRenderer() = default;

    VulkanRenderer(const VulkanRenderer &) = delete;
    VulkanRenderer &operator=(const VulkanRenderer &) = delete;
    VulkanRenderer(VulkanRenderer &&) = delete;
    VulkanRenderer &operator=(VulkanRenderer &&) = delete;

    auto recreate_resources() -> void;
    auto set_clear_color(const Color &color) -> void;
    auto render(const Scene &scene, const Camera &camera, ImDrawData *imgui_draw_data) -> void;

  private:
    auto prepare_frame_(const Scene &scene) -> std::vector<DrawItem>;
    auto record_(const std::vector<DrawItem> &draw_items, const Camera &camera, ImDrawData *imgui_draw_data = nullptr)
        -> void;
    auto end_frame_() -> void;

    std::uint32_t max_frames_in_flight_;

    const VulkanDevice &device_;
    const VulkanSurface &surface_;
    ResourceManager &resource_manager_;
    VulkanSwapchain swapchain_;
    VulkanFrameCommandContext frame_command_context_;
    std::vector<VulkanGpuBuffer> uniform_buffers_;
    DepthBuffer depth_buffer_;
    VulkanDescriptorPool descriptor_pool_;
    VulkanPipelineFactory pipeline_factory_;
    VulkanPipelineResources pipeline_resources_;
    std::vector<::vk::raii::DescriptorSet> descriptor_sets_;
    ::vk::ClearColorValue clear_color_;

    std::uint32_t current_swap_chain_image_index_{0};
    bool framebuffer_resized_ = false;

    static constexpr auto make_draw_sort_key_(
        std::uint64_t pipeline_id,
        MaterialHandle material_handle,
        MeshHandle mesh_handle,
        std::int32_t depth_bucket = 0) -> DrawSortKey;

    friend class ImguiWrapper;
    std::function<void()> imgui_resize_callback;
    inline auto set_imgui_resize_callback_(std::function<void()> fn) -> void
    {
        imgui_resize_callback = fn;
    }
};

}
