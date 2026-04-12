#pragma once

#include <cstdint>

namespace pong
{

struct Offset
{
    std::uint32_t x;
    std::uint32_t y;
};

struct Size
{
    std::uint32_t width;
    std::uint32_t height;
};

}
