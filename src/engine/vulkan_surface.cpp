#include "vulkan_surface.h"

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan_instance.h"
#include "platform/win32_window.h"

namespace pong
{

VulkanSurface::VulkanSurface(const VulkanInstance &vk_instance, const WindowHandles &handles)
    : surface_(vk_instance.get(), ::vk::Win32SurfaceCreateInfoKHR({}, handles.instance, handles.window))
{
}

auto VulkanSurface::get() const -> const ::vk::raii::SurfaceKHR &
{
    return surface_;
}

auto VulkanSurface::get_surface_capabilities(const ::vk::raii::PhysicalDevice &physical_device) const
    -> ::vk::SurfaceCapabilitiesKHR
{
    return physical_device.getSurfaceCapabilitiesKHR(surface_);
}

auto VulkanSurface::get_surface_formats(const ::vk::raii::PhysicalDevice &physical_device) const
    -> std::vector<::vk::SurfaceFormatKHR>
{
    return physical_device.getSurfaceFormatsKHR(surface_);
}

auto VulkanSurface::get_present_modes(const ::vk::raii::PhysicalDevice &physical_device) const
    -> std::vector<::vk::PresentModeKHR>
{
    return physical_device.getSurfacePresentModesKHR(surface_);
}

auto VulkanSurface::get_present_support(
    const ::vk::raii::PhysicalDevice &physical_device,
    std::uint32_t queue_family_index) const -> bool
{
    return physical_device.getSurfaceSupportKHR(queue_family_index, surface_);
}

}
