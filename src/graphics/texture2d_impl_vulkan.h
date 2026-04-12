#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_gpu_image.h"
#include "math/rectangle.h"

namespace pong

{

class VulkanDevice;

class Texture2DImpl_Vulkan
{
  public:
    Texture2DImpl_Vulkan(const VulkanDevice &device, std::string_view name, ::vk::Extent2D extent, ::vk::Format format);
    ~Texture2DImpl_Vulkan() = default;

    Texture2DImpl_Vulkan(const Texture2DImpl_Vulkan &) = delete;
    auto operator=(const Texture2DImpl_Vulkan &) -> Texture2DImpl_Vulkan & = delete;
    Texture2DImpl_Vulkan(Texture2DImpl_Vulkan &&) noexcept = default;
    auto operator=(Texture2DImpl_Vulkan &&) noexcept -> Texture2DImpl_Vulkan & = default;

    auto name() const -> std::string_view;
    auto width() const -> std::uint32_t;
    auto height() const -> std::uint32_t;

    auto image() const -> ::vk::Image;
    auto image_view() const -> ::vk::ImageView;
    auto sampler() const -> ::vk::Sampler;

  private:
    std::string name_;
    ::vk::Extent2D extent_;
    VulkanGpuImage image_;
}; // class Texture2DImpl_Vulkan

} // namespace pong
