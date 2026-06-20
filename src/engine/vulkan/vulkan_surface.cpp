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
    : surface_{nullptr}
{
    arm::log::debug("VulkanSurface constructor");

    const auto surface_create_info = ::vk::Win32SurfaceCreateInfoKHR{
        .sType = ::vk::StructureType::eWin32SurfaceCreateInfoKHR,
        .pNext = nullptr,
        .flags = {},
        .hinstance = handles.instance,
        .hwnd = handles.window,
    };

    auto surface_result = check_vk_expected(vk_instance.get().createWin32SurfaceKHR(surface_create_info));
    if (!surface_result.has_value())
    {
        throw arm::Exception("unable to create Vulkan surface");
    }
    surface_ = std::move(surface_result.value());
}

auto VulkanSurface::get() const -> const ::vk::raii::SurfaceKHR &
{
    return surface_;
}

auto VulkanSurface::native_handle() const -> const ::vk::SurfaceKHR &
{
    return *surface_;
}

}
