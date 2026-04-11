#pragma once

#include <cstddef>

#include <vulkan/vulkan_raii.hpp>

namespace pong
{

class VulkanDevice;

class GpuImage
{
  public:
    GpuImage(const VulkanDevice &device, ::vk::Extent2D extent, ::vk::Format format);
    ~GpuImage() = default;

    // movable but non-copyable or move assignable
    GpuImage(const GpuImage &) = delete;
    auto operator=(const GpuImage &) -> GpuImage & = delete;
    GpuImage(GpuImage &&) noexcept = default;
    auto operator=(GpuImage &&) noexcept -> GpuImage & = default;

    auto image() const -> ::vk::Image;
    auto image_view() const -> ::vk::ImageView;
    auto sampler() const -> ::vk::Sampler;
    auto extent() const -> ::vk::Extent2D;
    auto format() const -> ::vk::Format;

  private:
    const VulkanDevice *device_;
    ::vk::raii::Image image_;
    ::vk::raii::DeviceMemory memory_;
    ::vk::raii::ImageView view_;
    ::vk::raii::Sampler sampler_;
    ::vk::Extent2D extent_;
    ::vk::Format format_;

}; // class GpuImage

} // namespace pong
