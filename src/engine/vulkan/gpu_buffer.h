#pragma once

#include <cstddef>

#include <vulkan/vulkan_raii.hpp>

namespace pong
{
class VulkanDevice;

class GpuBuffer
{
  public:
    GpuBuffer(
        const VulkanDevice &device,
        ::vk::DeviceSize size,
        ::vk::BufferUsageFlags usage,
        ::vk::MemoryPropertyFlags memory_flags);
    ~GpuBuffer() = default;

    GpuBuffer(GpuBuffer &&) noexcept = default;
    GpuBuffer &operator=(GpuBuffer &&) noexcept = default;
    GpuBuffer(const GpuBuffer &) = delete;
    GpuBuffer &operator=(const GpuBuffer &) = delete;

    auto map() -> void *;
    auto unmap() -> void;
    auto upload(const void *data, std::size_t bytes, std::size_t offset = 0) -> void;

    auto native_handle() const -> ::vk::Buffer;
    auto size() const -> ::vk::DeviceSize;

  private:
    const VulkanDevice &device_;
    ::vk::DeviceSize size_{0};
    ::vk::raii::DeviceMemory memory_;
    ::vk::raii::Buffer buffer_;
    ::vk::MemoryPropertyFlags memory_flags_;
};

}
