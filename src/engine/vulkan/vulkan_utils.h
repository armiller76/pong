#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "graphics/shader.h"
#include "utils/error.h"
#include "utils/exception.h"


namespace pong
{

static inline auto check_vk_result(VkResult result) -> void
{
    if (result == VK_SUCCESS)
    {
        return;
    }
    else
    {
        auto trace = std::stacktrace::current(2zu);
        if (result > 0)
        {
            arm::log::warn(
                "Vulkan non-fatal error: {}\nStacktrace:\n{}\nContinuing",
                ::vk::to_string(static_cast<::vk::Result>(result)),
                trace);
        }
        else
        {
            arm::log::error(
                "Vulkan fatal error: {}\nStacktrace:\n{}\nTerminating",
                ::vk::to_string(static_cast<::vk::Result>(result)),
                trace);
            std::terminate();
            std::unreachable();
        }
    }
}

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
