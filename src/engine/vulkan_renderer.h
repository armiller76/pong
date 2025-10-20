#pragma once

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan_command_context.h"
#include "engine/vulkan_surface.h"
#include "engine/vulkan_swapchain.h"

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

  private:
    const VulkanDevice &device_;
    const VulkanSurface &surface_;
    VulkanSwapchain swapchain_;
    VulkanCommandContext command_context_;

    std::uint32_t current_image_index_{0};
    bool framebuffer_resized_ = false;
};

}
