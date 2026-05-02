#pragma once

#include <cstdint>
#include <optional>
#include <tuple>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "core/resource_handles.h"
#include "engine/vulkan/vulkan_depth_buffer.h"
#include "engine/vulkan/vulkan_descriptor_pool.h"
#include "engine/vulkan/vulkan_frame_command_context.h"
#include "engine/vulkan/vulkan_gpu_buffer.h"
#include "engine/vulkan/vulkan_pipeline_types.h"
#include "engine/vulkan/vulkan_swapchain.h"
#include "graphics/color.h"
#include "graphics/mesh.h"

struct ImDrawData;

namespace pong
{

class Camera;
class ResourceLoader;
class ResourceManager;
class Scene;
class VulkanDevice;
class VulkanDescriptorPool;
class VulkanPipelineManager;
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
        const ResourceManager &resource_manager,
        VulkanPipelineManager &pipeline_manager,
        VulkanDescriptorPool &descriptor_pool,
        std::uint32_t max_frames_in_flight,
        const Color clear_color = {0.42f, 0.42f, 0.42f, 1.0f});
    ~VulkanRenderer() = default;

    VulkanRenderer(const VulkanRenderer &) = delete;
    VulkanRenderer &operator=(const VulkanRenderer &) = delete;
    VulkanRenderer(VulkanRenderer &&) = delete;
    VulkanRenderer &operator=(VulkanRenderer &&) = delete;

    auto recreate_resources() -> void;
    auto needs_recreate() const -> bool;

    auto swapchain_image_count() const -> std::uint32_t;
    auto swapchain_image_index() const -> std::uint32_t;
    auto swapchain_format() const -> ::vk::Format;

    auto descriptor_pool() -> VulkanDescriptorPool *;
    auto set_clear_color(const Color &color) -> void;

    auto render(const Scene &scene, ImDrawData *imgui_draw_data) -> void;

    auto shutdown() -> void;

  private:
    std::uint32_t max_frames_in_flight_;

    const VulkanDevice &device_;
    const ResourceManager &resource_manager_;
    VulkanPipelineManager &pipeline_manager_;
    VulkanDescriptorPool &descriptor_pool_;
    VulkanSwapchain swapchain_;
    std::vector<VulkanGpuBuffer> camera_uniform_buffers_;
    std::vector<VulkanGpuBuffer> light_uniform_buffers_;
    VulkanFrameCommandContext frame_command_context_;
    DepthBuffer depth_buffer_;
    std::vector<::vk::raii::DescriptorSet> per_frame_descriptor_sets_;
    ::vk::ClearColorValue clear_color_;
    std::uint32_t current_swap_chain_image_index_{0};
    std::uint64_t frame_counter_{0}; // TODO use this
    bool needs_recreate_ = false;

  private:
    auto init_() -> void;

    auto prepare_frame_(const Scene &scene) -> RenderStatus;
    auto record_(const Scene &scene, const std::vector<DrawItem> &draw_items, ImDrawData *imgui_draw_data = nullptr)
        -> void;
    auto end_frame_() -> void;

    static constexpr auto make_draw_sort_key_(
        PipelineKey pipeline_key,
        std::optional<MaterialHandle> material_handle,
        MeshHandle mesh_handle,
        std::int32_t depth_bucket = 0) -> DrawSortKey;
};

}
