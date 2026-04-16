#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "core/resource_handles.h"
#include "mesh.h"

namespace pong
{

struct Renderable
{
    MeshHandle mesh_handle;
    std::optional<MaterialHandle> material_handle;
};

struct Model
{
    std::string name;
    std::vector<Renderable> renderables;
};

} // namespace pong
