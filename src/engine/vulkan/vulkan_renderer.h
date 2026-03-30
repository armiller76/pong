#pragma once

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>

#include "engine/resource_manager.h"
#include "graphics/color.h"
#include "vulkan_command_context.h"
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
        const Color clear_color = {0.5f, 1.0f, 0.0f, 1.0f});
    // TODO rule of 5
    ~VulkanRenderer() = default;

    VulkanRenderer(const VulkanRenderer &) = delete;
    VulkanRenderer &operator=(VulkanRenderer &&) = delete;
    VulkanRenderer(VulkanRenderer &&) = delete;
    VulkanRenderer &operator=(const VulkanRenderer &&) = delete;

    auto begin_frame() -> void;
    auto end_frame() -> void;

    auto framebuffer_resized() -> void;
    auto set_clear_color(const Color &color) -> void;

  private:
    const VulkanDevice &device_;
    const VulkanSurface &surface_;
    const ResourceManager &resource_manager_;
    VulkanSwapchain swapchain_;
    VulkanCommandContext command_context_;
    VulkanPipelineFactory pipeline_factory_;
    VulkanPipelineResources pipeline_resources_;

    ::vk::ClearColorValue clear_color_;

    std::uint32_t current_image_index_{0};
    bool framebuffer_resized_ = false;

    // TODO this is where you left off when you went gallavanting off to write a resource/asset manager
    auto load_shaders_() -> void;
};

}
