#include "vulkan_descriptor_pool.h"

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>

#include "utils/log.h"
#include "vulkan_device.h"

namespace pong
{

VulkanDescriptorPool::VulkanDescriptorPool(const VulkanDevice &device, const std::uint32_t max_frames_in_flight)
    : device_{device}
    , max_frames_{max_frames_in_flight}
    , pool_{create_pool_()}
{
    arm::log::debug("VulkanDescriptorPool constructor");
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

    return device_.get().createDescriptorPool(pool_create_info);
}

}
