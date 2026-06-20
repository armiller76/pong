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

    [[nodiscard]] auto format() const -> ::vk::Format;
    [[nodiscard]] auto image() const -> ::vk::Image;
    [[nodiscard]] auto image_view() const -> ::vk::ImageView;

  private:
    ::vk::Format format_;
    ::vk::raii::Image image_;
    ::vk::raii::DeviceMemory memory_;
    ::vk::raii::ImageView view_;
};

}
