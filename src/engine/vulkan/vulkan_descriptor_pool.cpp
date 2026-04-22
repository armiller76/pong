#include "vulkan_descriptor_pool.h"

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "engine/ubo.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_gpu_buffer.h"
#include "utils/error.h"
#include "utils/log.h"

namespace pong
{

VulkanDescriptorPool::VulkanDescriptorPool(
    const VulkanDevice &device,
    const std::vector<VulkanGpuBuffer> &view_proj_uniform_buffers,
    const std::uint32_t max_frames_in_flight)
    : device_{device}
    , view_proj_uniform_buffers_{view_proj_uniform_buffers}
    , frames_in_flight_{max_frames_in_flight}
    , pool_{create_pool_()}
{
    arm::log::debug("VulkanDescriptorPool constructor");
}

auto VulkanDescriptorPool::native_handle() const -> ::vk::DescriptorPool
{
    return *pool_;
}

auto VulkanDescriptorPool::allocate_per_frame_descriptor_sets(const ::vk::raii::DescriptorSetLayout &layout)
    -> std::vector<vk::raii::DescriptorSet>
{
    auto layout_array = std::vector(frames_in_flight_, *layout);
    auto descriptor_set_allocate_info = ::vk::DescriptorSetAllocateInfo{};
    descriptor_set_allocate_info.sType = ::vk::StructureType::eDescriptorSetAllocateInfo;
    descriptor_set_allocate_info.pNext = nullptr;
    descriptor_set_allocate_info.descriptorPool = *pool_;
    descriptor_set_allocate_info.descriptorSetCount = static_cast<std::uint32_t>(layout_array.size());
    descriptor_set_allocate_info.pSetLayouts = layout_array.data();

    auto descriptor_sets = device_.native_handle().allocateDescriptorSets(descriptor_set_allocate_info);

    for (std::size_t i = 0; i < frames_in_flight_; ++i)
    {
        auto view_proj_descriptor_buffer_info = ::vk::DescriptorBufferInfo{};
        view_proj_descriptor_buffer_info.buffer = view_proj_uniform_buffers_.at(i).native_handle();
        view_proj_descriptor_buffer_info.offset = 0;
        view_proj_descriptor_buffer_info.range = sizeof(UBO_ViewProj);

        auto view_proj_write_descriptor_set = ::vk::WriteDescriptorSet{};
        view_proj_write_descriptor_set.sType = ::vk::StructureType::eWriteDescriptorSet;
        view_proj_write_descriptor_set.pNext = nullptr;
        view_proj_write_descriptor_set.dstSet = descriptor_sets.at(i);
        view_proj_write_descriptor_set.dstBinding = 0;
        view_proj_write_descriptor_set.dstArrayElement = 0;
        view_proj_write_descriptor_set.descriptorCount = 1;
        view_proj_write_descriptor_set.descriptorType = ::vk::DescriptorType::eUniformBuffer;
        view_proj_write_descriptor_set.pImageInfo = nullptr;
        view_proj_write_descriptor_set.pBufferInfo = &view_proj_descriptor_buffer_info;
        view_proj_write_descriptor_set.pTexelBufferView = nullptr;

        auto descriptors = std::array{
            view_proj_write_descriptor_set,
        };

        device_.native_handle().updateDescriptorSets(descriptors, {});
    }

    return descriptor_sets;
} // allocate_descriptor_sets

auto VulkanDescriptorPool::allocate_material_descriptor_set(const ::vk::raii::DescriptorSetLayout &layout)
    -> vk::raii::DescriptorSet
{
    auto descriptor_set_allocate_info = ::vk::DescriptorSetAllocateInfo{};
    descriptor_set_allocate_info.sType = ::vk::StructureType::eDescriptorSetAllocateInfo;
    descriptor_set_allocate_info.pNext = nullptr;
    descriptor_set_allocate_info.descriptorPool = *pool_;
    descriptor_set_allocate_info.descriptorSetCount = 1u;
    descriptor_set_allocate_info.pSetLayouts = &*layout;
    auto result = device_.native_handle().allocateDescriptorSets(descriptor_set_allocate_info);
    arm::ensure(result.size() > 0, "allocateDescriptorSets returned empty vector");
    return std::move(result[0]);
}

auto VulkanDescriptorPool::create_pool_() -> ::vk::raii::DescriptorPool
{
    // TODO MAX_MATERIALS is a magic number!
    const auto ubo_count = PER_FRAME_UBO_COUNT * frames_in_flight_ + MATERIAL_UBO_COUNT * MAX_MATERIALS;
    const auto sampler_count = MAX_MATERIALS * SAMPLERS_PER_MATERIAL;

    auto ubo_pool_size = ::vk::DescriptorPoolSize{};
    ubo_pool_size.type = ::vk::DescriptorType::eUniformBuffer;
    ubo_pool_size.descriptorCount = ubo_count;

    auto sampler_pool_size = ::vk::DescriptorPoolSize{};
    sampler_pool_size.type = ::vk::DescriptorType::eCombinedImageSampler;
    sampler_pool_size.descriptorCount = sampler_count;

    auto pool_sizes = std::array{
        ubo_pool_size,
        sampler_pool_size,
    };

    auto pool_create_info = ::vk::DescriptorPoolCreateInfo{};
    pool_create_info.sType = ::vk::StructureType::eDescriptorPoolCreateInfo;
    pool_create_info.pNext = nullptr;
    pool_create_info.flags = ::vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    pool_create_info.maxSets = frames_in_flight_ + MAX_MATERIALS;
    pool_create_info.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size());
    pool_create_info.pPoolSizes = pool_sizes.data();

    return device_.native_handle().createDescriptorPool(pool_create_info);
} // create_pool_

} // namespace pong
