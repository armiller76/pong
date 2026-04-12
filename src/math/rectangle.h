#pragma once

#include <cstdint>

#include "utils/log.h"

namespace pong
{

struct Offset2D
{
    std::int32_t x;
    std::int32_t y;

    Offset2D()
        : x{0}
        , y{0}
    {
    }

    Offset2D(std::int32_t x, std::int32_t y)
        : x{x}
        , y{y}
    {
    }
};

struct Extent2D
{
    std::uint32_t width;
    std::uint32_t height;

    Extent2D(std::uint32_t w, std::uint32_t h)
        : width{w}
        , height{h}
    {
        if (w == 0 || h == 0)
        {
            arm::log::warn("Extent2D with zero component w={} h={}", w, h);
        }
    }
};

struct Rectangle
{
    Offset2D offset;
    Extent2D extent;
};

} // namespace pong
