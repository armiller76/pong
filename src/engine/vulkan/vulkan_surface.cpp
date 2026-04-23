#include "vulkan_surface.h"

#include <vulkan/vulkan_raii.hpp>

#include "platform/win32_window.h"
#include "utils/log.h"
#include "vulkan_instance.h"

namespace pong
{

VulkanSurface::VulkanSurface(const VulkanInstance &vk_instance, const Win32WindowHandles &handles)
    : surface_(vk_instance.native_handle(), ::vk::Win32SurfaceCreateInfoKHR({}, handles.instance, handles.window))
{
    arm::log::debug("VulkanSurface constructor");
}

auto VulkanSurface::get() const -> const ::vk::raii::SurfaceKHR &
{
    return surface_;
}

auto VulkanSurface::native_handle() const -> const ::vk::SurfaceKHR &
{
    return *surface_;
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
