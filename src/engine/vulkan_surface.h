#pragma once

#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "platform/win32_window.h"

namespace pong
{
class VulkanInstance;

class VulkanSurface
{
  public:
    VulkanSurface(const VulkanInstance &vk_instance, const WindowHandles &handles);

    auto get() const -> const ::vk::raii::SurfaceKHR &;

    auto get_surface_capabilities(const ::vk::raii::PhysicalDevice &physical_device) const
        -> ::vk::SurfaceCapabilitiesKHR;
    auto get_surface_formats(const ::vk::raii::PhysicalDevice &physical_device) const
        -> std::vector<::vk::SurfaceFormatKHR>;
    auto get_present_modes(const ::vk::raii::PhysicalDevice &physical_device) const
        -> std::vector<::vk::PresentModeKHR>;
    auto get_present_support(const ::vk::raii::PhysicalDevice &physical_device, std::uint32_t queue_family_index) const
        -> bool;

  private:
    ::vk::raii::SurfaceKHR surface_;
};

}
