#pragma once

#include <cstdint>

#include "utils/log.h"

namespace pong
{

struct Offset2D
{
    constexpr Offset2D()
        : Offset2D(0u, 0u)
    {
    }

    constexpr Offset2D(std::int32_t x, std::int32_t y)
        : x{x}
        , y{y}
    {
    }

    constexpr auto operator==(const Offset2D &other) const -> bool = default;

    std::int32_t x;
    std::int32_t y;
};

struct Extent2D
{
    constexpr Extent2D()
        : Extent2D(100u, 100u)
    {
    }

    constexpr Extent2D(std::uint32_t w, std::uint32_t h)
        : width{w}
        , height{h}
    {
        if (w == 0 || h == 0)
        {
            arm::log::warn("Extent2D with zero component w={} h={}", w, h);
        }
    }

    constexpr auto operator==(const Extent2D &other) const -> bool = default;

    std::uint32_t width;
    std::uint32_t height;
};

// default construct gives you offset(0,0) and extent(100,100)
struct Rectangle
{
    Offset2D offset;
    Extent2D extent;
};

} // namespace pong
