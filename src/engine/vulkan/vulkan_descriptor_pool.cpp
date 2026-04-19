#include "vulkan_descriptor_pool.h"

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "engine/ubo.h"
#include "utils/log.h"
#include "vulkan_device.h"
#include "vulkan_gpu_buffer.h"

namespace pong
{

VulkanDescriptorPool::VulkanDescriptorPool(
    const VulkanDevice &device,
    const std::vector<VulkanGpuBuffer> &view_proj_uniform_buffers,
    const std::vector<VulkanGpuBuffer> &material_uniform_buffers,
    const std::uint32_t max_frames_in_flight)
    : device_{device}
    , view_proj_uniform_buffers_{view_proj_uniform_buffers}
    , material_uniform_buffers_{material_uniform_buffers}
    , frames_in_flight_{max_frames_in_flight}
    , pool_{create_pool_()}
{
    arm::log::debug("VulkanDescriptorPool constructor");
}

auto VulkanDescriptorPool::native_handle() const -> ::vk::DescriptorPool
{
    return *pool_;
}

auto VulkanDescriptorPool::allocate_descriptor_sets(
    const ::vk::DescriptorSetLayout &layout,
    std::uint32_t frames_in_flight) -> std::vector<vk::raii::DescriptorSet>
{
    auto layouts = std::vector<::vk::DescriptorSetLayout>(frames_in_flight, layout);

    auto descriptor_set_allocate_info = ::vk::DescriptorSetAllocateInfo{};
    descriptor_set_allocate_info.sType = ::vk::StructureType::eDescriptorSetAllocateInfo;
    descriptor_set_allocate_info.pNext = nullptr;
    descriptor_set_allocate_info.descriptorPool = *pool_;
    descriptor_set_allocate_info.descriptorSetCount = static_cast<std::uint32_t>(layouts.size());
    descriptor_set_allocate_info.pSetLayouts = layouts.data();

    auto descriptor_sets = device_.native_handle().allocateDescriptorSets(descriptor_set_allocate_info);

    for (std::size_t i = 0; i < frames_in_flight; ++i)
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

        auto material_descriptor_buffer_info = ::vk::DescriptorBufferInfo{};
        material_descriptor_buffer_info.buffer = material_uniform_buffers_.at(i).native_handle();
        material_descriptor_buffer_info.offset = 0;
        material_descriptor_buffer_info.range = sizeof(UBO_Material);

        auto material_write_descriptor_set = ::vk::WriteDescriptorSet{};
        material_write_descriptor_set.sType = ::vk::StructureType::eWriteDescriptorSet;
        material_write_descriptor_set.pNext = nullptr;
        material_write_descriptor_set.dstSet = descriptor_sets.at(i);
        material_write_descriptor_set.dstBinding = 2;
        material_write_descriptor_set.dstArrayElement = 0;
        material_write_descriptor_set.descriptorCount = 1;
        material_write_descriptor_set.descriptorType = ::vk::DescriptorType::eUniformBuffer;
        material_write_descriptor_set.pImageInfo = nullptr;
        material_write_descriptor_set.pBufferInfo = &material_descriptor_buffer_info;
        material_write_descriptor_set.pTexelBufferView = nullptr;

        auto descriptors = std::array{
            view_proj_write_descriptor_set,
            material_write_descriptor_set,
        };

        device_.native_handle().updateDescriptorSets(descriptors, {});
    }

    return descriptor_sets;
} // allocate_descriptor_sets

auto VulkanDescriptorPool::create_pool_() -> ::vk::raii::DescriptorPool
{
    auto view_matrix_pool_size = ::vk::DescriptorPoolSize{};
    view_matrix_pool_size.type = ::vk::DescriptorType::eUniformBuffer;
    view_matrix_pool_size.descriptorCount = frames_in_flight_;

    auto sampler_pool_size = ::vk::DescriptorPoolSize{};
    sampler_pool_size.type = ::vk::DescriptorType::eCombinedImageSampler;
    sampler_pool_size.descriptorCount = frames_in_flight_;

    auto material_pool_size = ::vk::DescriptorPoolSize{};
    material_pool_size.type = ::vk::DescriptorType::eUniformBuffer;
    material_pool_size.descriptorCount = frames_in_flight_;

    auto pool_sizes = std::array{
        view_matrix_pool_size,
        sampler_pool_size,
        material_pool_size,
    };

    auto pool_create_info = ::vk::DescriptorPoolCreateInfo{};
    pool_create_info.sType = ::vk::StructureType::eDescriptorPoolCreateInfo;
    pool_create_info.pNext = nullptr;
    pool_create_info.flags = ::vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    pool_create_info.maxSets = frames_in_flight_;
    pool_create_info.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size());
    pool_create_info.pPoolSizes = pool_sizes.data();

    return device_.native_handle().createDescriptorPool(pool_create_info);
} // create_pool_

} // namespace pong
