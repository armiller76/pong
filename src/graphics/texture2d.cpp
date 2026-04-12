#include "texture2d.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_utils.h"
#include "image.h"
#include "math/rectangle.h"
#include "texture2d_impl_vulkan.h"

namespace pong
{

// vulkan specific constructor overload
Texture2D::Texture2D(std::string_view name, Extent2D extent, ImageFormat format, const VulkanDevice &device)
{
    impl_ = std::make_unique<Texture2DImpl_Vulkan>(device, name, to_vk(extent), to_vk(format));
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

} // namespace pong
