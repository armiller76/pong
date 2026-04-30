#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "graphics/vertex.h"

namespace pong
{

struct MeshView
{
    std::span<const Vertex> vertices;
    std::span<const std::uint32_t> indices;
};

struct MeshViewMikk
{
    std::span<const Vertex> vertices;
    std::span<const std::uint32_t> indices;
    std::vector<::glm::vec4> tangents;
};

} // namespace pong
