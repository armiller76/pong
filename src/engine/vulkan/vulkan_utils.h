#pragma once

// #include <vulkan/vulkan_raii.hpp>

#include "graphics/shader.h"
#include "utils/exception.h"

namespace pong
{

inline auto to_vk_stage(ShaderStage stage) -> ::vk::ShaderStageFlagBits
{
    using enum ShaderStage;
    switch (stage)
    {
        case Vertex: return ::vk::ShaderStageFlagBits::eVertex;
        case Fragment: return ::vk::ShaderStageFlagBits::eFragment;
        case Compute: return ::vk::ShaderStageFlagBits::eCompute;
        default: throw arm::Exception("unknown shader stage");
    }
}

//[[nodiscard]] inline auto create_vk_shader_module(const vk::raii::Device &device, const Shader &shader)
//    -> ::vk::raii::ShaderModule
//{
//    return device.createShaderModule(
//        {{}, shader.spirv.size(), reinterpret_cast<const uint32_t *>(shader.spirv.data())});
//}

}
