#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace pong
{

class Image;
class VulkanDevice;
class VulkanImmediateCommandContext;

class VulkanGpuImage
{
  public:
    VulkanGpuImage(const VulkanDevice &device, ::vk::Extent2D extent, ::vk::Format format);
    ~VulkanGpuImage() = default;

    // movable but non-copyable
    VulkanGpuImage(const VulkanGpuImage &) = delete;
    auto operator=(const VulkanGpuImage &) -> VulkanGpuImage & = delete;
    VulkanGpuImage(VulkanGpuImage &&) noexcept = default;
    auto operator=(VulkanGpuImage &&) noexcept -> VulkanGpuImage & = default;

    auto image() const -> ::vk::Image;
    auto image_view() const -> ::vk::ImageView;
    auto sampler() const -> ::vk::Sampler;
    auto extent() const -> ::vk::Extent2D;
    auto format() const -> ::vk::Format;

    auto upload(VulkanImmediateCommandContext &command_context, const Image &image) -> void;

  private:
    const VulkanDevice *device_;
    ::vk::Extent2D extent_;
    ::vk::Format format_;
    ::vk::raii::Image image_;
    ::vk::raii::DeviceMemory memory_;
    ::vk::raii::ImageView view_;
    ::vk::raii::Sampler sampler_;

}; // class VulkanGpuImage

} // namespace pong
