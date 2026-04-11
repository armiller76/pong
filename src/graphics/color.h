#pragma once

#include <algorithm>
#include <format>
#include <string>

#include "utils/log.h"

namespace pong
{

struct Color
{
    float r;
    float g;
    float b;
    float a;

    Color(float r, float g, float b, float a)
    {
        this->r = std::clamp(r, 0.0f, 1.0f);
        this->g = std::clamp(g, 0.0f, 1.0f);
        this->b = std::clamp(b, 0.0f, 1.0f);
        this->a = std::clamp(a, 0.0f, 1.0f);
    }
};

inline auto to_string(const Color &color) -> std::string
{
    return std::format("r={}, g={}, b={}, a={}", color.r, color.g, color.b, color.a);
}

}
