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

    Color(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f)
    {
        this->r = std::clamp(r, 0.0f, 1.0f);
        this->g = std::clamp(g, 0.0f, 1.0f);
        this->b = std::clamp(b, 0.0f, 1.0f);
        this->a = std::clamp(a, 0.0f, 1.0f);
    }

    constexpr auto operator==(const Color &other) const -> bool = default;

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

template <>
struct std::hash<pong::Color>
{
    auto operator()(const pong::Color &c) const noexcept -> std::size_t
    {
        auto result = std::size_t{42};
        const auto hash_combine = [&result](const std::size_t value)
        {
            // boost-style mix to preserve entropy from each field hash
            result ^= value + static_cast<std::size_t>(0x9e3779b97f4a7c15ull) + (result << 6u) + (result >> 2u);
        };
        hash_combine(std::hash<float>{}(c.r));
        hash_combine(std::hash<float>{}(c.g));
        hash_combine(std::hash<float>{}(c.b));
        hash_combine(std::hash<float>{}(c.a));

        return result;
    }
};
