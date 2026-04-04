#pragma once

#include <cstdint>
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
    std::vector<std::uint32_t> spirv;
    ShaderStage stage;

    // Shader(std::string_view name, std::vector<std::uint32_t> spirv, ShaderStage stage)
    //     : name{std::format("shader_{}", name)}
    //     , spirv{std::move(spirv)}
    //     , stage{stage}
    //{
    // }

    auto spirv_view() const -> std::span<const std::uint32_t>
    {
        return {spirv.data(), spirv.data() + spirv.size()};
    }
};

}
