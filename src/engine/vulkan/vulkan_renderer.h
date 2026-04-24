#pragma once

#include <cstdint>
#include <functional>
#include <tuple>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "core/resource_handles.h"
#include "engine/resource_loader.h"
#include "engine/resource_manager.h"
#include "engine/vulkan/vulkan_depth_buffer.h"
#include "engine/vulkan/vulkan_descriptor_pool.h"
#include "engine/vulkan/vulkan_frame_command_context.h"
#include "engine/vulkan/vulkan_gpu_buffer.h"
#include "engine/vulkan/vulkan_pipeline_factory.h"
#include "engine/vulkan/vulkan_swapchain.h"
#include "graphics/color.h"
#include "graphics/mesh.h"

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

    enum class RenderStatusCode
    {
        ReadyToRecord,
        SkipMinimized,
        RecreateRequested,
        Error,
    };
    struct RenderStatus
    {
        RenderStatusCode code;
        std::vector<DrawItem> draw_items;
    };

  public:
    VulkanRenderer(
        const VulkanDevice &device,
        const VulkanSurface &surface,
        std::uint32_t max_frames_in_flight,
        const Color clear_color = {0.42f, 0.42f, 0.42f, 1.0f});
    ~VulkanRenderer() = default;

    VulkanRenderer(const VulkanRenderer &) = delete;
    VulkanRenderer &operator=(const VulkanRenderer &) = delete;
    VulkanRenderer(VulkanRenderer &&) = delete;
    VulkanRenderer &operator=(VulkanRenderer &&) = delete;

    auto shutdown() -> void;

    auto recreate_resources() -> bool;
    auto needs_recreate() -> bool;

    auto set_clear_color(const Color &color) -> void;

    auto load_scene(std::string_view filename) -> Scene;

    auto render(const Scene &scene, const Camera &camera, ImDrawData *imgui_draw_data) -> void;

  private:
    auto prepare_frame_(const Scene &scene) -> RenderStatus;
    auto record_(const std::vector<DrawItem> &draw_items, const Camera &camera, ImDrawData *imgui_draw_data = nullptr)
        -> void;
    auto end_frame_() -> void;
    auto handle_out_of_date_() -> void;

    std::uint32_t max_frames_in_flight_;

    const VulkanDevice &device_;
    const VulkanSurface &surface_;
    ResourceManager resource_manager_;
    ResourceLoader resource_loader_;
    VulkanSwapchain swapchain_;
    VulkanFrameCommandContext frame_command_context_;
    std::vector<VulkanGpuBuffer> view_proj_uniform_buffers_;
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
