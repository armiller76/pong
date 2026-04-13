#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "math/rectangle.h"
#include "utils/error.h"
#include "utils/log.h"

namespace pong
{
using namespace std::literals;

enum class ImageFormat
{
    RGBA8,
    RGB8,
    BGRA8,
    R8,
    RG8,
    RGBA16F,
    RGBA32F,
    RGBA8_sRGB,
    D16,
    D24,
    D32F,
    D24S8,
    D32FS8,
    BC1,
    BC2,
    BC3,
    BC4,
    BC5,
    BC6H,
    BC7,

    UNDEFINED,
};

class Image
{
  public:
    Image(std::string_view name, Extent2D extent, ImageFormat(format), std::span<const std::uint8_t> pixels)
        : name_{name}
        , extent_{extent}
        , format_{format}
        , pixels_{pixels.begin(), pixels.end()}
    {
        arm::log::debug("Image constructor {}", name);
        arm::ensure(!(extent_.width == 0 || extent_.height == 0), "Image created with zero height or width");
        arm::ensure(!(format_ == ImageFormat::UNDEFINED), "Unknown image format");
        arm::ensure(!pixels_.empty(), "Image created with no pixels");
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

inline auto bytes_per_pixel(ImageFormat f) -> std::uint32_t
{
    using enum ImageFormat;
    switch (f)
    {
        case R8: return 1u;

        case RG8: return 2u;

        case D24:
        case RGB8: return 3u;

        case D16:
        case D24S8:
        case BGRA8:
        case RGBA8:
        case RGBA8_sRGB: return 4u;

        case D32FS8:
        case RGBA16F: return 8u;

        case D32F:
        case RGBA32F: return 16u;

        case BC1:
        case BC2:
        case BC3:
        case BC4:
        case BC5:
        case BC6H:
        case BC7: return 0u;

        case UNDEFINED:
        default: throw arm::Exception("undefined ImageFormat in bytes_per_pixel");
    }
}

inline auto to_string(ImageFormat f) -> std::string_view
{
    using enum ImageFormat;
    switch (f)
    {
        case R8: return "R8"sv;
        case RG8: return "RG8"sv;
        case D24: return "D24"sv;
        case RGB8: return "RGB8"sv;
        case D16: return "D16"sv;
        case D24S8: return "D24S8"sv;
        case BGRA8: return "BGRA8"sv;
        case RGBA8: return "RGBA8"sv;
        case RGBA8_sRGB: return "RGBA8_sRGB"sv;
        case D32FS8: return "D32FS8"sv;
        case RGBA16F: return "RBGA16F"sv;
        case D32F: return "D32F"sv;
        case RGBA32F: return "RGBA32F"sv;
        case BC1: return "BC1"sv;
        case BC2: return "BC2"sv;
        case BC3: return "BC3"sv;
        case BC4: return "BC4"sv;
        case BC5: return "BC5"sv;
        case BC6H: return "BC6H"sv;
        case BC7: return "BC7"sv;

        case UNDEFINED: return "UNDEFINED"sv;
        default: throw arm::Exception("undefined ImageFormat in to_string");
    }
}

} // namespace pong
