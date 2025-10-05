#include "vulkan_swapchain.h"

#include <vulkan/vulkan_raii.hpp>

#include "platform/win32_window.h"

namespace pong
{

VulkanSurface::VulkanSurface(::vk::raii::Instance &vk_instance, WindowHandles handles)
    : surface_(vk_instance, ::vk::Win32SurfaceCreateInfoKHR{}.setHwnd(handles.window).setHinstance(handles.instance))
{
}

}
