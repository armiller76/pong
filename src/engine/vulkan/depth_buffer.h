#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace pong

{
class VulkanDevice;

class DepthBuffer
{
  public:
    DepthBuffer(const VulkanDevice &device, ::vk::Extent2D extent);
    ~DepthBuffer() = default;

    // moveable, non-copyable
    DepthBuffer(const DepthBuffer &) = delete;
    auto operator=(const DepthBuffer &) -> DepthBuffer & = delete;
    DepthBuffer(DepthBuffer &&) noexcept = default;
    auto operator=(DepthBuffer &&) noexcept -> DepthBuffer & = default;

    auto format() const -> ::vk::Format;
    auto image() const -> ::vk::Image;
    auto image_view() const -> ::vk::ImageView;

  private:
    const VulkanDevice *device_;
    ::vk::Format format_;
    ::vk::raii::Image image_;
    ::vk::raii::DeviceMemory memory_;
    ::vk::raii::ImageView view_;
};

}
