#include "vulkan_gpu_buffer.h"

#include <cstring>
#include <memory>
#include <utility>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_utils.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

VulkanGpuBuffer::VulkanGpuBuffer(
    const VulkanDevice &device,
    ::vk::DeviceSize size,
    ::vk::BufferUsageFlags usage,
    ::vk::MemoryPropertyFlags memory_flags)
    : device_{std::addressof(device)}
    , size_{size}
    , buffer_(
          [&]() -> vk::raii::Buffer
          {
              const auto buffer_info = ::vk::BufferCreateInfo{
                  .sType = ::vk::StructureType::eBufferCreateInfo,
                  .pNext = nullptr,
                  .flags = {},
                  .size = size,
                  .usage = usage,
                  .sharingMode = ::vk::SharingMode::eExclusive,
                  .queueFamilyIndexCount = 0u,
                  .pQueueFamilyIndices = nullptr,
              };

              auto buffer_result = check_vk_expected(device_->get().createBuffer(buffer_info));
              if (!buffer_result.has_value())
              {
                  throw arm::Exception("unable to create gpu buffer");
              }
              return std::move(buffer_result.value());
          }())
    , memory_(
          [&]() -> vk::raii::DeviceMemory
          {
              const auto memory_requirements = buffer_.getMemoryRequirements();
              const auto memory_info = ::vk::MemoryAllocateInfo{
                  .sType = ::vk::StructureType::eMemoryAllocateInfo,
                  .pNext = nullptr,
                  .allocationSize = memory_requirements.size,
                  .memoryTypeIndex = device_->find_memory_type_index(memory_requirements, memory_flags),
              };

              auto memory_result = check_vk_expected(device_->get().allocateMemory(memory_info));
              if (!memory_result.has_value())
              {
                  throw arm::Exception("unable to allocate buffer memory");
              }
              buffer_.bindMemory(*memory_result.value(), 0u);
              return std::move(memory_result.value());
          }())
    , memory_flags_{memory_flags}
{
    arm::log::debug("VulkanGpuBuffer Constructor: {} - {} bytes", ::vk::to_string(usage), size);
}

auto VulkanGpuBuffer::unmap() -> void
{
    memory_.unmapMemory();
}

auto VulkanGpuBuffer::upload(const void *data, std::size_t bytes, std::size_t offset) -> void
{
    arm::ensure(offset + bytes <= static_cast<std::size_t>(size_), "Gpu buffer upload out of bounds");
    arm::ensure(
        (memory_flags_ & ::vk::MemoryPropertyFlagBits::eHostVisible) == ::vk::MemoryPropertyFlagBits::eHostVisible,
        "upload to GPU requires HOST_VISIBLE memory");

    auto *dest = memory_.mapMemory(offset, bytes);
    std::memcpy(dest, data, bytes);
    memory_.unmapMemory();

    if (!((memory_flags_ & ::vk::MemoryPropertyFlagBits::eHostCoherent) == ::vk::MemoryPropertyFlagBits::eHostCoherent))
    {
        auto flush_range = ::vk::MappedMemoryRange{
            .sType = ::vk::StructureType::eMappedMemoryRange,
            .pNext = nullptr,
            .memory = *memory_,
            .offset = offset,
            .size = bytes,
        };
        device_->get().flushMappedMemoryRanges({flush_range});
    }
}

auto VulkanGpuBuffer::native_handle() const -> const ::vk::Buffer
{
    return *buffer_;
}

auto VulkanGpuBuffer::size() const -> ::vk::DeviceSize
{
    return size_;
}

} // namespace pong
