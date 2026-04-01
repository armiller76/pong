#include "gpu_buffer.h"

#include <cstring>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_device.h"
#include "utils/error.h"

namespace pong
{

GpuBuffer::GpuBuffer(
    const VulkanDevice &device,
    ::vk::DeviceSize size,
    ::vk::BufferUsageFlags usage,
    ::vk::MemoryPropertyFlags memory_flags)
    : device_(device)
    , size_(size)
    , memory_({})
    , buffer_({})
    , memory_flags_(memory_flags)
{
    auto buffer_info = ::vk::BufferCreateInfo{};
    buffer_info.size = size_;
    buffer_info.usage = usage;
    buffer_info.sharingMode = ::vk::SharingMode::eExclusive;

    buffer_ = device_.get().createBuffer(buffer_info);

    auto memory_requirements = buffer_.getMemoryRequirements();

    auto memory_info = ::vk::MemoryAllocateInfo{};
    memory_info.allocationSize = memory_requirements.size;
    memory_info.memoryTypeIndex = device_.find_memory_type_index(memory_requirements, memory_flags_);

    memory_ = device_.get().allocateMemory(memory_info);
    buffer_.bindMemory(*memory_, 0);
}

auto GpuBuffer::map() -> void *
{
    arm::ensure(
        (memory_flags_ & ::vk::MemoryPropertyFlagBits::eHostVisible) == ::vk::MemoryPropertyFlagBits::eHostVisible,
        "mapping requires HOST_VISIBLE memory");
    return memory_.mapMemory(0, size_);
}

auto GpuBuffer::unmap() -> void
{
    memory_.unmapMemory();
}

auto GpuBuffer::upload(const void *data, std::size_t bytes, std::size_t offset) -> void
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
        auto flush_range = ::vk::MappedMemoryRange{};
        flush_range.memory = *memory_;
        flush_range.offset = offset;
        flush_range.size = bytes;
        device_.get().flushMappedMemoryRanges({flush_range});
    }
}

auto GpuBuffer::native_handle() const -> ::vk::Buffer
{
    return *buffer_;
}

auto GpuBuffer::size() const -> ::vk::DeviceSize
{
    return size_;
}

}
