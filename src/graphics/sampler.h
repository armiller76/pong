#pragma once

#include <cstdint>
#include <utility>

#include "graphics/types.h"

namespace pong
{

struct SamplerKey
{
    FilterMode mag_filter_mode;
    FilterMode min_filter_mode;
    MipMapMode mip_map_mode;
    WrapMode u_wrap;
    WrapMode v_wrap;
    WrapMode w_wrap; // not used
    bool enable_anisotropy;
    float max_anisotropy;
    bool enable_compare;
    CompareOp compare_op;
    float min_lod;
    float max_lod;
    float mip_lod_bias;
    bool enable_border_color;
    bool unnormalized_coordinates;

    constexpr auto operator==(const SamplerKey &other) const -> bool = default;
}; // struct SamplerKey

} // namespace pong

template <>
struct std::hash<pong::SamplerKey>
{
    auto operator()(const pong::SamplerKey &k) const noexcept -> std::size_t
    {
        auto result = std::size_t{42};
        const auto hash_combine = [&result](const std::size_t value)
        {
            // boost-style mix to preserve entropy from each field hash
            result ^= value + static_cast<std::size_t>(0x9e3779b97f4a7c15ull) + (result << 6u) + (result >> 2u);
        };
        hash_combine(std::hash<std::uint32_t>{}(std::to_underlying(k.mag_filter_mode)));
        hash_combine(std::hash<std::uint32_t>{}(std::to_underlying(k.min_filter_mode)));
        hash_combine(std::hash<std::uint32_t>{}(std::to_underlying(k.mip_map_mode)));
        hash_combine(std::hash<std::uint32_t>{}(std::to_underlying(k.u_wrap)));
        hash_combine(std::hash<std::uint32_t>{}(std::to_underlying(k.v_wrap)));
        hash_combine(std::hash<std::uint32_t>{}(std::to_underlying(k.w_wrap)));
        hash_combine(std::hash<bool>{}(k.enable_anisotropy));
        hash_combine(std::hash<float>{}(k.max_anisotropy));
        hash_combine(std::hash<bool>{}(k.enable_compare));
        hash_combine(std::hash<std::uint32_t>{}(std::to_underlying(k.compare_op)));
        hash_combine(std::hash<float>{}(k.min_lod));
        hash_combine(std::hash<float>{}(k.max_lod));
        hash_combine(std::hash<float>{}(k.mip_lod_bias));
        hash_combine(std::hash<bool>{}(k.enable_border_color));
        hash_combine(std::hash<bool>{}(k.unnormalized_coordinates));

        return result;
    }
};
