#pragma once

#include <algorithm>
#include <cstdint>
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

    Color(float r, float g, float b, float a)
    {
        this->r = std::clamp(r, 0.0f, 1.0f);
        this->g = std::clamp(g, 0.0f, 1.0f);
        this->b = std::clamp(b, 0.0f, 1.0f);
        this->a = std::clamp(a, 0.0f, 1.0f);
    }

    static constexpr auto float_to_byte(float f) -> std::uint8_t
    {
        return std::round(std::clamp<float>(f, 0.0f, 1.0f) * 255.0f);
    }

    static constexpr auto float_to_srgb(float f) -> float
    {
        auto result = float{};
        if (f < 0.0031308f)
        {
            result = 12.92 * f;
        }
        else
        {
            result = 1.055 * std::pow(f, 1.0f / 2.4f) - 0.055;
        }
        return result;
    }

    static constexpr auto float_to_srgb_byte(float f) -> std::uint8_t
    {
        return float_to_byte(float_to_srgb(f));
    }
};

inline auto to_string(const Color &color) -> std::string
{
    return std::format("r={}, g={}, b={}, a={}", color.r, color.g, color.b, color.a);
}

} // namespace pong
