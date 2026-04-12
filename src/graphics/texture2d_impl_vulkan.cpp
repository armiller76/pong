#include "texture2d_impl_vulkan.h"

#include <cstdint>
#include <string>
#include <string_view>

#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_utils.h"
#include <vulkan/vulkan_raii.hpp>

namespace pong
{

Texture2DImpl_Vulkan::Texture2DImpl_Vulkan(
    const VulkanDevice &device,
    std::string_view name,
    ::vk::Extent2D extent,
    ::vk::Format format)
    : name_{name}
    , extent_{extent}
    , image_{device, extent_, format}
{
}

auto Texture2DImpl_Vulkan::name() const -> std::string_view
{
    return name_;
}

auto Texture2DImpl_Vulkan::width() const -> std::uint32_t
{
    return extent_.width;
}

auto Texture2DImpl_Vulkan::height() const -> std::uint32_t
{
    return extent_.height;
}

auto Texture2DImpl_Vulkan::image() const -> ::vk::Image
{
    return image_.image();
}

auto Texture2DImpl_Vulkan::image_view() const -> ::vk::ImageView
{
    return image_.image_view();
}

auto Texture2DImpl_Vulkan::sampler() const -> ::vk::Sampler
{
    return image_.sampler();
}

}
