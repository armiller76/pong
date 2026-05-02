#include "texture2d.h"

#include <cstdint>
#include <string_view>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_utils.h"
#include "image.h"

namespace pong
{

Texture2D::Texture2D(VulkanDevice &device, Image &image, const SamplerKey &sampler_key)
    : name_{image.name()}
    , extent_{to_vk(image.extent())}
    , sampler_{device.get_sampler(sampler_key)}
    , image_{device, extent_, sampler_, to_vk(image.format())}
{
}

auto Texture2D::name() const -> std::string_view
{
    return name_;
}

auto Texture2D::width() const -> std::uint32_t
{
    return extent_.width;
}

auto Texture2D::height() const -> std::uint32_t
{
    return extent_.height;
}

auto Texture2D::image() const -> ::vk::Image
{
    return image_.image();
}

auto Texture2D::image_view() const -> ::vk::ImageView
{
    return image_.image_view();
}

auto Texture2D::sampler() const -> ::vk::Sampler
{
    return image_.sampler();
}

auto Texture2D::upload_pixels(VulkanImmediateCommandContext &command_context, const Image &image) -> void
{
    image_.upload(command_context, image);
}

} // namespace pong
