#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "utils/hash.h"

namespace pong
{

enum class ShaderStage
{
    Vertex,
    Fragment,
    Compute
};

struct Shader
{
    std::string name;
    std::vector<std::byte> spirv;
    ShaderStage stage;
};

}
