#pragma once

#include <vector>

#include <vulkan/vulkan_raii.hpp>

namespace pong
{
class VulkanInstance;
struct Win32WindowHandles;

class VulkanSurface
{
  public:
    VulkanSurface(const VulkanInstance &vk_instance, const Win32WindowHandles &handles);

    VulkanSurface(VulkanSurface &&) noexcept = default;
    VulkanSurface &operator=(VulkanSurface &&) noexcept = default;
    VulkanSurface(const VulkanSurface &) = delete;
    VulkanSurface &operator=(const VulkanSurface &) = delete;
    ~VulkanSurface() = default;

    auto get() const -> const ::vk::raii::SurfaceKHR &;
    auto native_handle() const -> const ::vk::SurfaceKHR &;

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
