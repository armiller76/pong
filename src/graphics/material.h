#pragma once

#include <optional>
#include <string>

#include "core/resource_handles.h"
#include "graphics/glm_wrapper.h" // IWYU pragma: keep
#include "graphics/types.h"

namespace pong
{

struct Material
{
    std::string name;
    ::glm::vec4 base_color_factor;
    std::optional<Texture2DHandle> base_color_texture_handle;
    float metallic_factor;
    float roughness_factor;
    std::optional<Texture2DHandle> metallic_roughness_texture_handle;
    std::optional<Texture2DHandle> normal_texture_handle;
    AlphaMode alpha_mode;
}; // struct Material

} // namespace pong
