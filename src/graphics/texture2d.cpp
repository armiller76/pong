#include "texture2d.h"

#include <cstdint>
#include <memory>
#include <string_view>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_utils.h"
#include "image.h"
#include "texture2d_impl_vulkan.h"

namespace pong
{

// vulkan specific constructor overload
Texture2D::Texture2D(Image &image, const VulkanDevice &device)
{
    impl_ = std::make_unique<Texture2DImpl_Vulkan>(device, image.name(), to_vk(image.extent()), to_vk(image.format()));
}

// other specific constructor overload
// Texture2D::Texture2D(std::string_view name, Extent2D extent, ImageFormat format, const OtherAPIDevice &device)
//{
//    impl_ = std::make_unique<Texture2DImpl_OtherAPI>(device, name, to_otherAPI(extent), to_otherAPI(format));
//}

auto Texture2D::name() const -> std::string_view
{
    return impl_->name();
}

auto Texture2D::width() const -> std::uint32_t
{
    return impl_->width();
}

auto Texture2D::height() const -> std::uint32_t
{
    return impl_->height();
}

auto Texture2D::image_view() const -> ::vk::ImageView
{
    return impl_->image_view();
}

auto Texture2D::sampler() const -> ::vk::Sampler
{
    return impl_->sampler();
}

auto Texture2D::upload_pixels(VulkanImmediateCommandContext &command_context, const Image &image) -> void
{
    impl_->upload(command_context, image);
}

} // namespace pong
