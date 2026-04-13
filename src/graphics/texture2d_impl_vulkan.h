#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_gpu_image.h"
#include "math/rectangle.h"

namespace pong

{

class Image;
class VulkanDevice;
class VulkanImmediateCommandContext;

class Texture2DImpl_Vulkan
{
  public:
    Texture2DImpl_Vulkan(const VulkanDevice &device, std::string_view name, ::vk::Extent2D extent, ::vk::Format format)
        : name_{name}
        , extent_{extent}
        , image_{device, extent_, format}
    {
    }
    ~Texture2DImpl_Vulkan() = default;

    Texture2DImpl_Vulkan(const Texture2DImpl_Vulkan &) = delete;
    auto operator=(const Texture2DImpl_Vulkan &) -> Texture2DImpl_Vulkan & = delete;
    Texture2DImpl_Vulkan(Texture2DImpl_Vulkan &&) noexcept = default;
    auto operator=(Texture2DImpl_Vulkan &&) noexcept -> Texture2DImpl_Vulkan & = default;

    auto name() const -> std::string_view
    {
        return name_;
    }

    auto width() const -> std::uint32_t
    {
        return extent_.width;
    }

    auto height() const -> std::uint32_t
    {
        return extent_.height;
    }

    auto image() const -> ::vk::Image
    {
        return image_.image();
    }

    auto image_view() const -> ::vk::ImageView
    {
        return image_.image_view();
    }

    auto sampler() const -> ::vk::Sampler
    {
        return image_.sampler();
    }

    auto upload(VulkanImmediateCommandContext &command_context, const Image &image)
    {
        image_.upload(command_context, image);
    }

  private:
    std::string name_;
    ::vk::Extent2D extent_;
    VulkanGpuImage image_;
}; // class Texture2DImpl_Vulkan

} // namespace pong
