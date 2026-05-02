#pragma once

#include <cstdint>
#include <format>
#include <string>
#include <utility>

#include <vulkan/vulkan_raii.hpp>

#include "core/resource_handles.h"
#include "graphics/types.h"
#include "utils/exception.h"

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

    RasterStateCount,
};

enum class VertexInput
{
    NOT_IMPLEMENTED,
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

struct PipelineKey
{
    PassType pass_type;            // 8 bits
    AlphaMode alpha_mode;          // 8 bits
    RasterState raster_state;      // 8 bits
    VertexInput vertex_input;      // 8 bits
    ShaderFeature shader_features; // 32 bits

    auto operator==(const PipelineKey &other) const -> bool
    {
        return (
            pass_type == other.pass_type && alpha_mode == other.alpha_mode && raster_state == other.raster_state
            && vertex_input == other.vertex_input && shader_features == other.shader_features);
    }

    auto pack() -> std::uint64_t
    {
        auto result = std::uint64_t{};
        result |= (static_cast<std::uint64_t>(pass_type) & 0xFF) << 0;
        result |= (static_cast<std::uint64_t>(alpha_mode) & 0xFF) << 8;
        result |= (static_cast<std::uint64_t>(raster_state) & 0xFF) << 16;
        result |= (static_cast<std::uint64_t>(vertex_input) & 0xFF) << 24;
        result |= (static_cast<std::uint64_t>(shader_features) & 0xFFFFFFF) << 32;
        return result;
    }
};

struct PipelineEntry
{
    std::string name;
    PassType pass_type;
    PipelineKey pipeline_key;
    ShaderHandle vertex_shader;
    ShaderHandle fragment_shader;
    ::vk::raii::Pipeline pipeline;
    // vector of shader modules? individual shader modules?
    // other identifiers used to build this specific pipeline
    // debug name / stats / last_used
};

struct VulkanPipelineResources
{
    ::vk::raii::PipelineLayout layout;
    ::vk::raii::DescriptorSetLayout per_frame_descriptor_set_layout;
    ::vk::raii::DescriptorSetLayout per_material_descriptor_set_layout;
}; // struct VulkanPipelineResources

constexpr auto to_string(PassType pass_type) -> std::string
{
    switch (pass_type)
    {
        case PassType::Main: return "main";
        case PassType::Light: return "light";
        case PassType::Debug: return "debug";
        case PassType::PassTypeCount:
        default: throw arm::Exception("invalid PassType");
    }
}

constexpr auto to_string(RasterState raster_state) -> std::string
{
    switch (raster_state)
    {
        case RasterState::Default: return "default";
        case RasterState::Double: return "double";
        case RasterState::Wireframe: return "wireframe";
        case RasterState::RasterStateCount:
        default: throw arm::Exception("invalid RasterState");
    }
}

constexpr auto to_string(VertexInput vertex_input) -> std::string
{
    switch (vertex_input)
    {
        case VertexInput::NOT_IMPLEMENTED: return "notimplemented";
        case VertexInput::SkinnedMesh: return "skinnedmesh";
        case VertexInput::StaticMesh: return "staticmesh";
        case VertexInput::Debug: return "debug";
        case VertexInput::VertexInputCount:
        default: throw arm::Exception("invalid VertexInput");
    }
}

constexpr auto name_pipeline(const PipelineKey &key) -> std::string
{
    return std::format(
        "{}_{}_{}_{}_{:x}",
        to_string(key.pass_type),
        to_string(key.raster_state),
        to_string(key.alpha_mode),
        to_string(key.vertex_input),
        std::to_underlying(key.shader_features));
}

} // namespace pong

template <>
struct std::hash<pong::PipelineKey>
{
    auto operator()(pong::PipelineKey key) const noexcept -> std::size_t
    {
        return std::hash<std::uint64_t>{}(key.pack());
    }
};
