#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "platform/win32_window.h"

namespace pong
{

class VulkanSurface
{
  public:
    VulkanSurface(::vk::raii::Instance &vk_instance, WindowHandles handles);

  private:
    ::vk::raii::SurfaceKHR surface_;
};

}
