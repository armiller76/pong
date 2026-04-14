#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "image_format.h"
#include "math/rectangle.h"
#include "utils/error.h"
#include "utils/log.h"

namespace pong
{

class Image
{
  public:
    Image(std::string_view name, Extent2D extent, ImageFormat format, std::span<const std::uint8_t> pixels)
        : name_{name}
        , extent_{extent}
        , format_{format}
        , pixels_{pixels.begin(), pixels.end()}
    {
        arm::log::debug("Image constructor {}", name);
        arm::ensure(!(extent_.width == 0 || extent_.height == 0), "Image created with zero height or width");
        arm::ensure(!(format_ == ImageFormat::UNDEFINED), "Unknown image format");
        arm::ensure(!pixels_.empty(), "Image created with no pixels");
        arm::ensure(
            static_cast<std::size_t>(extent_.width) * extent_.height * bytes_per_pixel(format_) == pixels.size(),
            "Extent width * height does not equal pixel byte count");
    }

    auto name() const -> std::string_view
    {
        return name_;
    }

    auto extent() const -> Extent2D
    {
        return extent_;
    }

    auto format() const -> ImageFormat
    {
        return format_;
    }

    auto pixels() const -> std::span<const std::uint8_t>
    {
        return pixels_;
    }

  private:
    std::string name_;
    Extent2D extent_;
    ImageFormat format_;
    std::vector<std::uint8_t> pixels_;
}; // class Image

} // namespace pong
