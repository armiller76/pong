#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace pong
{
class VulkanInstance;
struct Win32WindowHandles;

class VulkanSurface
{
  public:
    VulkanSurface(const VulkanInstance &vk_instance, const Win32WindowHandles &handles);

    VulkanSurface(const VulkanSurface &) = delete;
    auto operator=(const VulkanSurface &) -> VulkanSurface & = delete;
    VulkanSurface(VulkanSurface &&) noexcept = default;
    auto operator=(VulkanSurface &&) noexcept -> VulkanSurface & = default;

    [[nodiscard]] auto get() const -> const ::vk::raii::SurfaceKHR &;
    [[nodiscard]] auto native_handle() const -> const ::vk::SurfaceKHR &;

  private:
    ::vk::raii::SurfaceKHR surface_;
};

}
