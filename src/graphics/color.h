#pragma once

#include <format>
#include <string>

namespace pong
{

struct Color
{
    float r;
    float g;
    float b;
    float a;
};

inline auto to_string(const Color &color) -> std::string
{
    return std::format("r={}, g={}, b={}, a={}", color.r, color.g, color.b, color.a);
}

}
