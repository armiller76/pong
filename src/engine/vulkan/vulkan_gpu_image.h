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
    VulkanGpuImage(const VulkanDevice &device, ::vk::Extent2D extent, ::vk::Sampler sampler, ::vk::Format format);
    ~VulkanGpuImage() = default;

    VulkanGpuImage(const VulkanGpuImage &) = delete;
    auto operator=(const VulkanGpuImage &) -> VulkanGpuImage & = delete;
    VulkanGpuImage(VulkanGpuImage &&) noexcept = default;
    auto operator=(VulkanGpuImage &&) noexcept -> VulkanGpuImage & = delete;

    [[nodiscard]] auto image() const -> ::vk::Image;
    [[nodiscard]] auto image_view() const -> ::vk::ImageView;
    [[nodiscard]] auto sampler() const -> ::vk::Sampler;
    [[nodiscard]] auto extent() const -> ::vk::Extent2D;
    [[nodiscard]] auto format() const -> ::vk::Format;

    auto upload(VulkanImmediateCommandContext &command_context, const Image &image) -> void;

  private:
    const VulkanDevice *device_;
    const ::vk::Extent2D extent_;
    const ::vk::Format format_;
    const ::vk::Sampler sampler_;
    ::vk::raii::Image image_;
    ::vk::raii::DeviceMemory memory_;
    ::vk::raii::ImageView view_;

}; // class VulkanGpuImage

} // namespace pong
