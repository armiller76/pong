#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace pong
{

struct alignas(16) UBO_Camera
{
    ::glm::mat4 view;
    ::glm::mat4 proj;
    ::glm::vec4 camera; // .xyz = position, .w = unused/pad
};

static_assert(sizeof(UBO_Camera) == 64 * 2 + 16, "ubo is incorrect size");
static_assert(alignof(UBO_Camera) == 16, "ubo is misaligned");
static_assert(offsetof(UBO_Camera, view) == 0, "offset is incorrect");
static_assert(offsetof(UBO_Camera, proj) == 64, "offset is incorrect");
static_assert(offsetof(UBO_Camera, camera) == 128, "offset is incorrect");

struct alignas(16) DirectionalLightData
{
    ::glm::vec4 direction_intensity; // .xyz = direction, .w = intensity
    ::glm::vec4 color;               // .xyz = color, .w = unused/pad
};

static_assert(sizeof(DirectionalLightData) == 16 * 2, "ubo is incorrect size");
static_assert(alignof(DirectionalLightData) == 16, "ubo is misaligned");
static_assert(offsetof(DirectionalLightData, direction_intensity) == 0, "offset is incorrect");
static_assert(offsetof(DirectionalLightData, color) == 16, "offset is incorrect");

struct alignas(16) UBO_Lighting
{
    ::glm::vec4 ambient_color_strength; // .xyz = color, .w = strength
    ::glm::uvec4 light_counts;          // .x = direction, .y = point, .z = spot, .w = unused/pad
    DirectionalLightData directional[MAX_DIR_LIGHTS];
};

static_assert(sizeof(UBO_Lighting) == 16 * 2 + MAX_DIR_LIGHTS * sizeof(DirectionalLightData), "ubo is incorrect size");
static_assert(alignof(UBO_Lighting) == 16, "ubo is misaligned");
static_assert(offsetof(UBO_Lighting, ambient_color_strength) == 0, "offset is incorrect");
static_assert(offsetof(UBO_Lighting, light_counts) == 16, "offset is incorrect");
static_assert(offsetof(UBO_Lighting, directional) == 32, "offset is incorrect");

struct alignas(16) UBO_Material
{
    ::glm::vec4 base_color_factor;
    float metallic_factor;
    float roughness_factor;
    float pad1;
    float pad2;
};

static_assert(sizeof(UBO_Material) == (16 * 1) + (4 * 4), "ubo is incorrect size");
static_assert(alignof(UBO_Material) == 16, "ubo is misaligned");
static_assert(offsetof(UBO_Material, base_color_factor) == 0, "offset is incorrect");
static_assert(offsetof(UBO_Material, metallic_factor) == 16, "offset is incorrect");
static_assert(offsetof(UBO_Material, roughness_factor) == 20, "offset is incorrect");

} // namespace pong
