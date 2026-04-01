#include "vulkan_descriptor_pool.h"

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "engine/ubo.h"
#include "gpu_buffer.h"
#include "utils/log.h"
#include "vulkan_device.h"

namespace pong
{

VulkanDescriptorPool::VulkanDescriptorPool(
    const VulkanDevice &device,
    const std::vector<GpuBuffer> &uniform_buffers,
    const std::uint32_t max_frames_in_flight)
    : device_{device}
    , uniform_buffers_{uniform_buffers}
    , max_frames_{max_frames_in_flight}
    , pool_{create_pool_()}
{
    arm::log::debug("VulkanDescriptorPool constructor");
}

auto VulkanDescriptorPool::native_handle() const -> ::vk::DescriptorPool
{
    return *pool_;
}

auto VulkanDescriptorPool::create_pool_() -> ::vk::raii::DescriptorPool
{
    auto vertex_pool_size = ::vk::DescriptorPoolSize{};
    vertex_pool_size.type = ::vk::DescriptorType::eUniformBuffer;
    vertex_pool_size.descriptorCount = max_frames_;

    auto pool_sizes = std::array{
        vertex_pool_size,
    };

    auto pool_create_info = ::vk::DescriptorPoolCreateInfo{};
    pool_create_info.sType = ::vk::StructureType::eDescriptorPoolCreateInfo;
    pool_create_info.pNext = nullptr;
    pool_create_info.flags = ::vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    pool_create_info.maxSets = max_frames_;
    pool_create_info.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size());
    pool_create_info.pPoolSizes = pool_sizes.data();

    return device_.native_handle().createDescriptorPool(pool_create_info);
}

auto VulkanDescriptorPool::allocate_descriptor_sets(
    const ::vk::DescriptorSetLayout &layout,
    std::uint32_t max_frames_in_flight) -> std::vector<vk::raii::DescriptorSet>
{
    auto layouts = std::vector<::vk::DescriptorSetLayout>(max_frames_in_flight, layout);

    auto descriptor_set_allocate_info = ::vk::DescriptorSetAllocateInfo{};
    descriptor_set_allocate_info.sType = ::vk::StructureType::eDescriptorSetAllocateInfo;
    descriptor_set_allocate_info.pNext = nullptr;
    descriptor_set_allocate_info.descriptorPool = *pool_;
    descriptor_set_allocate_info.descriptorSetCount = static_cast<std::uint32_t>(layouts.size());
    descriptor_set_allocate_info.pSetLayouts = layouts.data();

    auto descriptor_sets = device_.native_handle().allocateDescriptorSets(descriptor_set_allocate_info);

    for (std::size_t i = 0; i < max_frames_in_flight; ++i)
    {
        auto ubo_mvp_descriptor_buffer_info = ::vk::DescriptorBufferInfo{};
        ubo_mvp_descriptor_buffer_info.buffer = uniform_buffers_.at(i).native_handle();
        ubo_mvp_descriptor_buffer_info.offset = 0;
        ubo_mvp_descriptor_buffer_info.range = sizeof(ubo_mvp);

        auto ubo_write_descriptor_set = ::vk::WriteDescriptorSet{};
        ubo_write_descriptor_set.sType = ::vk::StructureType::eWriteDescriptorSet;
        ubo_write_descriptor_set.pNext = nullptr;
        ubo_write_descriptor_set.dstSet = descriptor_sets.at(i);
        ubo_write_descriptor_set.dstBinding = 0;
        ubo_write_descriptor_set.dstArrayElement = 0;
        ubo_write_descriptor_set.descriptorCount = 1;
        ubo_write_descriptor_set.descriptorType = ::vk::DescriptorType::eUniformBuffer;
        ubo_write_descriptor_set.pImageInfo = nullptr;
        ubo_write_descriptor_set.pBufferInfo = &ubo_mvp_descriptor_buffer_info;
        ubo_write_descriptor_set.pTexelBufferView = nullptr;

        auto descriptors = std::array{
            ubo_write_descriptor_set,
        };

        device_.native_handle().updateDescriptorSets(descriptors, {});
    }

    return descriptor_sets;
} // allocate_descriptor_sets

} // namespace pong
