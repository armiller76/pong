#pragma once

#include <memory>
#include <span>
#include <string>
#include <string_view>

#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_gpu_image.h"
#include "math/rectangle.h"
#include "texture2d_impl_vulkan.h"

namespace pong
{

class Image;
class VulkanImmediateCommandContext;

class Texture2D
{
  public:
    Texture2D(Image &image, const VulkanDevice &device);
    ~Texture2D() = default;

    Texture2D(const Texture2D &) = delete;
    auto operator=(const Texture2D &) -> Texture2D & = delete;
    Texture2D(Texture2D &&) noexcept = default;
    auto operator=(Texture2D &&) noexcept -> Texture2D & = default;

    auto name() const -> std::string_view;
    auto width() const -> std::uint32_t;
    auto height() const -> std::uint32_t;

    // TODO consider the pixel upload path returning pixels_uploaded or some confirmation of success?
    auto upload_pixels(VulkanImmediateCommandContext &command_context, const Image &image) -> void;

  private:
    std::unique_ptr<Texture2DImpl_Vulkan> impl_;
    // TODO consider an is_ready or is_uploaded bool to track if actual texture image has been uploaded to GPU

}; // class Texture

} // namespace pong
