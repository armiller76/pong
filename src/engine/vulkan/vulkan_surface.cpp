#include "vulkan_surface.h"

#include <utility>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_instance.h"
#include "engine/vulkan/vulkan_utils.h"
#include "platform/win32_window.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

VulkanSurface::VulkanSurface(const VulkanInstance &vk_instance, const Win32WindowHandles &handles)
    : surface_{[&]
               {
                   auto surface_create_info = ::vk::Win32SurfaceCreateInfoKHR{
                       .sType = ::vk::StructureType::eWin32SurfaceCreateInfoKHR,
                       .pNext = nullptr,
                       .flags = {},
                       .hinstance = handles.instance,
                       .hwnd = handles.window};
                   auto surface_result =
                       check_vk_expected(vk_instance.get().createWin32SurfaceKHR(surface_create_info));
                   if (!surface_result.has_value())
                   {
                       throw arm::Exception("unable to create Vulkan surface");
                   }
                   return std::move(surface_result.value());
               }()}
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
