#pragma once

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>

namespace pong
{
class VulkanInstance;
class VulkanSurface;

class VulkanDevice
{
  public:
    VulkanDevice(const VulkanInstance &instance, const VulkanSurface &surface);

    auto get() const -> const ::vk::raii::Device &;
    auto graphics_queue_index() const -> std::uint32_t;
    auto present_queue_index() const -> std::uint32_t;

  private:
    ::vk::raii::PhysicalDevice physical_device_;
    ::vk::raii::Device device_;
    std::uint32_t graphics_queue_family_index_;
    std::uint32_t present_queue_family_index_;
};

}
