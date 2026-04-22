#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_gpu_buffer.h"

namespace pong
{

class VulkanDescriptorPool
{
  public:
    VulkanDescriptorPool(
        const VulkanDevice &device,
        const std::vector<VulkanGpuBuffer> &view_proj_uniform_buffers,
        const std::uint32_t max_frames_in_flight);
    ~VulkanDescriptorPool() = default;

    VulkanDescriptorPool(const VulkanDescriptorPool &) = delete;
    auto operator=(const VulkanDescriptorPool &) -> VulkanDescriptorPool & = delete;
    VulkanDescriptorPool(VulkanDescriptorPool &&) noexcept = default;
    auto operator=(VulkanDescriptorPool &&) noexcept -> VulkanDescriptorPool & = delete;

    auto native_handle() const -> ::vk::DescriptorPool;

    auto allocate_per_frame_descriptor_sets(const ::vk::raii::DescriptorSetLayout &layout)
        -> std::vector<vk::raii::DescriptorSet>;
    auto allocate_material_descriptor_set(const ::vk::raii::DescriptorSetLayout &layout) -> vk::raii::DescriptorSet;

  private:
    auto create_pool_() -> ::vk::raii::DescriptorPool;

    const VulkanDevice &device_;
    const std::vector<VulkanGpuBuffer> &view_proj_uniform_buffers_;
    std::uint32_t frames_in_flight_;
    ::vk::raii::DescriptorPool pool_;

    static const std::uint32_t MAX_MATERIALS = 100u;
    static const std::uint32_t SAMPLERS_PER_MATERIAL = 3u; // base, metal/rough, normal,
    static const std::uint32_t PER_FRAME_UBO_COUNT = 1u;   // view/proj data,
    static const std::uint32_t MATERIAL_UBO_COUNT = 1u;    // one material factor,
};

}
