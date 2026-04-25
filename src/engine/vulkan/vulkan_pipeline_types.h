#pragma once

#include <utility>

#include <vulkan/vulkan_raii.hpp>

#include "graphics/types.h"

namespace pong
{

enum class PassType
{
    Main,
    Light,
    Debug,
    // Shadow, // future expansion
    // Depth,  // future expansion

    PassTypeCount,
};

enum class RasterState
{
    Default,   // back culling, filled polygons
    Double,    // no culling, filled polygons
    Wireframe, // no culling, lines only
    // ShadowCaster, // future expansion - front culling, filled polygons

    CullModeCount,
};

enum class VertexInput
{
    StaticMesh,
    SkinnedMesh,
    Debug,

    VertexInputCount,
};

enum class ShaderFeature : std::uint32_t
{
    None = 0,
    NormalMap = 1 << 0,
    AlphaTest = 1 << 1,
};

struct PipelineKey
{
    PassType pass_type;
    AlphaMode alpha_mode;
    RasterState raster_state;
    VertexInput vertex_input;
    ShaderFeature shader_features;
};

struct PipelineEntry
{
    ::vk::raii::Pipeline pipeline;
    // vector of shader modules? individual shader modules?
    // other identifiers used to build this specific pipeline
    // debug name / stats / last_used
};

constexpr auto operator|(ShaderFeature a, ShaderFeature b) noexcept -> ShaderFeature
{
    return static_cast<ShaderFeature>(std::to_underlying(a) | std::to_underlying(b));
}

constexpr auto operator&(ShaderFeature a, ShaderFeature b) noexcept -> ShaderFeature
{
    return static_cast<ShaderFeature>(std::to_underlying(a) & std::to_underlying(b));
}

constexpr auto operator|=(ShaderFeature &a, ShaderFeature b) noexcept -> ShaderFeature
{
    a = a | b;
    return a;
}

constexpr auto any(ShaderFeature a) noexcept -> bool
{
    return std::to_underlying(a) != 0;
}

// usage: bits = full bitfield, bit = bit to check
constexpr auto has(ShaderFeature bits, ShaderFeature bit) noexcept -> bool
{
    return any(bits & bit);
}

} // namespace pong
