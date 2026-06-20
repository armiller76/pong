#include "vulkan_descriptor_pool.h"

#include <cstdint>
#include <utility>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "engine/ubo.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_gpu_buffer.h"
#include "engine/vulkan/vulkan_gpu_image.h"
#include "engine/vulkan/vulkan_utils.h"
#include "utils/error.h"
#include "utils/log.h"

namespace pong
{

VulkanDescriptorPool::VulkanDescriptorPool(const VulkanDevice &device, const std::uint32_t max_frames_in_flight)
    : device_{device}
    , frames_in_flight_{max_frames_in_flight}
    , pool_{create_pool_()}
{
    arm::log::debug("VulkanDescriptorPool constructor");
}

auto VulkanDescriptorPool::native_handle() const -> ::vk::DescriptorPool
{
    return *pool_;
}

auto VulkanDescriptorPool::allocate_per_frame_descriptor_sets(
    const ::vk::raii::DescriptorSetLayout &layout,
    std::vector<VulkanGpuBuffer> &view_proj_uniform_buffers,
    std::vector<VulkanGpuBuffer> &light_uniform_buffers) -> std::vector<vk::raii::DescriptorSet>
{
    auto layouts = std::vector(frames_in_flight_, *layout);

    const auto descriptor_set_allocate_info = ::vk::DescriptorSetAllocateInfo{
        .sType = ::vk::StructureType::eDescriptorSetAllocateInfo,
        .pNext = nullptr,
        .descriptorPool = *pool_,
        .descriptorSetCount = static_cast<std::uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data(),
    };

    auto descriptor_set_result = check_vk_expected(device_.get().allocateDescriptorSets(descriptor_set_allocate_info));
    if (!descriptor_set_result.has_value())
    {
        throw arm::Exception("unable to allocate per-frame descriptor sets");
    }

    for (std::size_t i = 0; i < frames_in_flight_; ++i)
    {
        const auto view_proj_descriptor_buffer_info = ::vk::DescriptorBufferInfo{
            .buffer = view_proj_uniform_buffers[i].native_handle(),
            .offset = 0u,
            .range = sizeof(UBO_Camera),
        };
        const auto view_proj_write_descriptor_set = ::vk::WriteDescriptorSet{
            .sType = ::vk::StructureType::eWriteDescriptorSet,
            .pNext = nullptr,
            .dstSet = descriptor_set_result.value()[i],
            .dstBinding = 0u,
            .dstArrayElement = 0u,
            .descriptorCount = 1u,
            .descriptorType = ::vk::DescriptorType::eUniformBuffer,
            .pImageInfo = nullptr,
            .pBufferInfo = &view_proj_descriptor_buffer_info,
            .pTexelBufferView = nullptr,
        };

        const auto light_descriptor_buffer_info = ::vk::DescriptorBufferInfo{
            .buffer = light_uniform_buffers[i].native_handle(),
            .offset = 0u,
            .range = sizeof(UBO_Lighting),
        };
        const auto light_write_descriptor_set = ::vk::WriteDescriptorSet{
            .sType = ::vk::StructureType::eWriteDescriptorSet,
            .pNext = nullptr,
            .dstSet = descriptor_set_result.value()[i],
            .dstBinding = 1u,
            .dstArrayElement = 0u,
            .descriptorCount = 1u,
            .descriptorType = ::vk::DescriptorType::eUniformBuffer,
            .pImageInfo = nullptr,
            .pBufferInfo = &light_descriptor_buffer_info,
            .pTexelBufferView = nullptr,
        };

        const auto descriptors = std::array{
            view_proj_write_descriptor_set,
            light_write_descriptor_set,
        };

        device_.get().updateDescriptorSets(descriptors, {});
    }

    return std::move(descriptor_set_result.value());
} // allocate_descriptor_sets

auto VulkanDescriptorPool::allocate_material_descriptor_set(const ::vk::raii::DescriptorSetLayout &layout)
    -> vk::raii::DescriptorSet
{
    const auto descriptor_set_allocate_info = ::vk::DescriptorSetAllocateInfo{
        .sType = ::vk::StructureType::eDescriptorSetAllocateInfo,
        .pNext = nullptr,
        .descriptorPool = *pool_,
        .descriptorSetCount = 1u,
        .pSetLayouts = &*layout,
    };

    auto descriptor_set_result = check_vk_expected(device_.get().allocateDescriptorSets(descriptor_set_allocate_info));
    if (!descriptor_set_result.has_value())
    {
        throw arm::Exception("unalbe to allocate material descriptor set");
    }
    arm::ensure(descriptor_set_result.value().size() > 0, "allocateDescriptorSets returned empty vector");
    return std::move(descriptor_set_result.value()[0]);
}

auto VulkanDescriptorPool::create_pool_() -> ::vk::raii::DescriptorPool
{
    // TODO MAX_MATERIALS is a magic number!
    // IMPORTANT: set counts in header
    const auto ubo_count = PER_FRAME_UBO_COUNT * frames_in_flight_ + MATERIAL_UBO_COUNT * MAX_MATERIALS;
    const auto sampler_count = MAX_MATERIALS * SAMPLERS_PER_MATERIAL;

    const auto ubo_pool_size = ::vk::DescriptorPoolSize{
        .type = ::vk::DescriptorType::eUniformBuffer,
        .descriptorCount = ubo_count,
    };

    const auto sampler_pool_size = ::vk::DescriptorPoolSize{
        .type = ::vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = sampler_count,
    };

    const auto pool_sizes = std::array{
        ubo_pool_size,
        sampler_pool_size,
    };

    const auto pool_create_info = ::vk::DescriptorPoolCreateInfo{
        .sType = ::vk::StructureType::eDescriptorPoolCreateInfo,
        .pNext = nullptr,
        .flags = ::vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = frames_in_flight_ + MAX_MATERIALS,
        .poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };

    auto create_result = check_vk_expected(device_.get().createDescriptorPool(pool_create_info));
    if (!create_result)
    {
        throw arm::Exception("unable to create descriptor pool");
    }
    return std::move(create_result.value());
} // create_pool_

} // namespace pong
