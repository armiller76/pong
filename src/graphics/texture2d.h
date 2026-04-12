#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_gpu_image.h"
#include "image.h"
#include "math/rectangle.h"
#include "texture2d_impl_vulkan.h"


namespace pong
{

class Texture2D
{
  public:
    Texture2D(std::string_view name, Extent2D extent, ImageFormat format, const VulkanDevice &device);
    ~Texture2D() = default;

    Texture2D(const Texture2D &) = delete;
    auto operator=(const Texture2D &) -> Texture2D & = delete;
    Texture2D(Texture2D &&) noexcept = default;
    auto operator=(Texture2D &&) noexcept -> Texture2D & = default;

    auto name() const -> std::string_view;
    auto width() const -> std::uint32_t;
    auto height() const -> std::uint32_t;

  private:
    std::unique_ptr<Texture2DImpl_Vulkan> impl_;

}; // class Texture

} // namespace pong
