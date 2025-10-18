#pragma once

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>

namespace pong
{

class VulkanInstance;
class VulkanSurface;

struct VulkanDeviceInfo
{
    const vk::raii::PhysicalDevice *physical_device;
    std::uint32_t score;
    std::uint32_t graphics_index;
    std::uint32_t present_index;
    bool supports_api13;
    bool supports_dynamic_rendering;
    bool supports_sync2;
};

class VulkanDevice
{
  public:
    VulkanDevice(const VulkanInstance &instance, const VulkanSurface &surface);

    auto get() const -> const ::vk::raii::Device &;
    auto physical_device() const -> const ::vk::raii::PhysicalDevice &;

    auto graphics_queue() const -> ::vk::Queue;
    auto graphics_queue_index() const -> std::uint32_t;
    auto present_queue() const -> ::vk::Queue;
    auto present_queue_index() const -> std::uint32_t;

    auto supports_dynamic_rendering() const -> bool;

  private:
    ::vk::raii::PhysicalDevice physical_device_;
    ::vk::raii::Device device_;
    std::uint32_t graphics_queue_family_index_;
    std::uint32_t present_queue_family_index_;
    ::vk::Queue graphics_queue_;
    ::vk::Queue present_queue_;
    bool supports_api13_;
    bool supports_dynamic_rendering_;
    bool supports_sync2_;

    auto score_device(VulkanDeviceInfo &info, const VulkanSurface &surface) -> void;
};

}
