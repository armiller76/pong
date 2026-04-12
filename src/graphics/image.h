#pragma once

#include <cstdint>

namespace pong
{

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

}
