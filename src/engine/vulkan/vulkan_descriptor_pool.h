#pragma once

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>

#include "vulkan_device.h"

namespace pong
{

class VulkanDescriptorPool
{
  public:
    VulkanDescriptorPool(const VulkanDevice &device, const std::uint32_t max_frames_in_flight);
    ~VulkanDescriptorPool() = default;

    VulkanDescriptorPool(const VulkanDescriptorPool &) = delete;
    VulkanDescriptorPool &operator=(VulkanDescriptorPool &&) = delete;

    VulkanDescriptorPool(VulkanDescriptorPool &&) = default;
    VulkanDescriptorPool &operator=(const VulkanDescriptorPool &&) = default;

  private:
    auto create_pool_() -> ::vk::raii::DescriptorPool;

    const VulkanDevice &device_;
    std::uint32_t max_frames_;
    ::vk::raii::DescriptorPool pool_;
};

}
