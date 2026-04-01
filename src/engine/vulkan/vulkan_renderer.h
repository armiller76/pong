#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "engine/resource_manager.h"
#include "gpu_buffer.h"
#include "graphics/color.h"
#include "graphics/mesh.h"
#include "vulkan_command_context.h"
#include "vulkan_descriptor_pool.h"
#include "vulkan_layout_transition.h"
#include "vulkan_pipeline_factory.h"
#include "vulkan_surface.h"
#include "vulkan_swapchain.h"

namespace pong
{
class VulkanDevice;

class VulkanRenderer
{
  public:
    VulkanRenderer(
        const VulkanDevice &device,
        const VulkanSurface &surface,
        const ResourceManager &resource_manager,
        const std::uint32_t max_frames_in_flight,
        const Color clear_color = {0.5f, 1.0f, 0.0f, 1.0f});
    ~VulkanRenderer() = default;

    VulkanRenderer(const VulkanRenderer &) = delete;
    VulkanRenderer &operator=(VulkanRenderer &&) = delete;
    VulkanRenderer(VulkanRenderer &&) = delete;
    VulkanRenderer &operator=(const VulkanRenderer &&) = delete;

    auto framebuffer_resized() -> void;
    auto set_clear_color(const Color &color) -> void;
    auto render(const Mesh &mesh /*Scene &scene, Camera &camera, etc.*/) -> void;

  private:
    auto prepare_frame_() -> void;
    auto record_(const Mesh &mesh) -> void;
    auto end_frame_() -> void;
    auto transition_(std::uint32_t swap_chain_image_index, transition_info info) -> void;

    std::uint32_t max_frames_in_flight_;

    const VulkanDevice &device_;
    const VulkanSurface &surface_;
    const ResourceManager &resource_manager_;
    VulkanSwapchain swapchain_;
    VulkanCommandContext command_context_;
    std::vector<GpuBuffer> uniform_buffers_;
    VulkanDescriptorPool descriptor_pool_;
    VulkanPipelineFactory pipeline_factory_;
    VulkanPipelineResources pipeline_resources_;
    std::vector<::vk::raii::DescriptorSet> descriptor_sets_;
    ::vk::ClearColorValue clear_color_;

    std::uint32_t current_swap_chain_image_index_{0};
    bool framebuffer_resized_ = false;
};

}
