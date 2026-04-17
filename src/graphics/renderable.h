#pragma once

#include <optional>

#include "core/resource_handles.h"

namespace pong
{

struct Renderable
{
    MeshHandle mesh_handle;
    std::optional<MaterialHandle> material_handle;
};

}
