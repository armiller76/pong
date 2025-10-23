#pragma once

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan_command_context.h"
#include "engine/vulkan_surface.h"
#include "engine/vulkan_swapchain.h"
#include "graphics/color.h"

namespace pong
{
class VulkanDevice;

class VulkanRenderer
{
  public:
    VulkanRenderer(const VulkanDevice &device, const VulkanSurface &surface);

    auto begin_frame() -> void;
    auto end_frame() -> void;

    auto framebuffer_resized() -> void;
    auto set_clear_color(const Color &color) -> void;

  private:
    const VulkanDevice &device_;
    const VulkanSurface &surface_;
    VulkanSwapchain swapchain_;
    VulkanCommandContext command_context_;

    ::vk::raii::PipelineLayout graphics_pipeline_layout_;
    ::vk::raii::Pipeline graphics_pipeline_;

    ::vk::ClearColorValue clear_color_{};

    std::uint32_t current_image_index_{0};
    bool framebuffer_resized_ = false;
};

}
