#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "vulkan_device.h"
#include "vulkan_gpu_buffer.h"


namespace pong
{

class VulkanDescriptorPool
{
  public:
    VulkanDescriptorPool(
        const VulkanDevice &device,
        const std::vector<GpuBuffer> &uniform_buffers,
        const std::uint32_t max_frames_in_flight);
    ~VulkanDescriptorPool() = default;

    VulkanDescriptorPool(const VulkanDescriptorPool &) = delete;
    VulkanDescriptorPool &operator=(const VulkanDescriptorPool &) = delete;
    VulkanDescriptorPool(VulkanDescriptorPool &&) = default;
    VulkanDescriptorPool &operator=(VulkanDescriptorPool &&) = delete;

    auto native_handle() const -> ::vk::DescriptorPool;

    auto allocate_descriptor_sets(const ::vk::DescriptorSetLayout &layout, std::uint32_t max_frames_in_flight)
        -> std::vector<vk::raii::DescriptorSet>;

  private:
    auto create_pool_() -> ::vk::raii::DescriptorPool;

    const VulkanDevice &device_;
    const std::vector<GpuBuffer> &uniform_buffers_;
    std::uint32_t frames_in_flight_;
    ::vk::raii::DescriptorPool pool_;
};

}
