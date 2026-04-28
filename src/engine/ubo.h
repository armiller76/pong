#pragma once

#include "graphics/glm_wrapper.h" // IWYU pragma: keep

namespace pong
{

struct UBO_ViewProj
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

static_assert(sizeof(UBO_ViewProj) == 64 * 2, "ubo is incorrect size");
static_assert(alignof(UBO_ViewProj) == 16, "ubo is misaligned");
static_assert(offsetof(UBO_ViewProj, view) == 0, "offset is incorrect");
static_assert(offsetof(UBO_ViewProj, proj) == 64, "offset is incorrect");

struct UBO_Lighting
{
    alignas(16)::glm::vec4 direction_intensity;
    alignas(16)::glm::vec4 color_strength;
};

static_assert(sizeof(UBO_Lighting) == 16 * 2, "ubo is incorrect size");
static_assert(alignof(UBO_Lighting) == 16, "ubo is misaligned");
static_assert(offsetof(UBO_Lighting, direction_intensity) == 0, "offset is incorrect");
static_assert(offsetof(UBO_Lighting, color_strength) == 16, "offset is incorrect");

struct UBO_Material
{
    alignas(16)::glm::vec4 base_color_factor;
    alignas(4) float metallic_factor;
    alignas(4) float roughness_factor;
    alignas(4) float pad1;
    alignas(4) float pad2;
};

static_assert(sizeof(UBO_Material) == (16 * 1) + (4 * 4), "ubo is incorrect size");
static_assert(alignof(UBO_Material) == 16, "ubo is misaligned");
static_assert(offsetof(UBO_Material, base_color_factor) == 0, "offset is incorrect");
static_assert(offsetof(UBO_Material, metallic_factor) == 16, "offset is incorrect");
static_assert(offsetof(UBO_Material, roughness_factor) == 20, "offset is incorrect");

} // namespace pong
