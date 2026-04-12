#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "graphics/shader.h"
#include "math/rectangle.h"
#include "utils/error.h"
#include "utils/exception.h"

namespace pong
{

static inline auto check_vk_result(VkResult r) -> void
{
    if (r == VK_SUCCESS)
    {
        return;
    }
    else
    {
        auto trace = std::stacktrace::current(2zu);
        if (r > 0)
        {
            arm::log::warn(
                "Vulkan non-fatal error: {}\nStacktrace:\n{}\nContinuing",
                ::vk::to_string(static_cast<::vk::Result>(r)),
                trace);
        }
        else
        {
            arm::log::error(
                "Vulkan fatal error: {}\nStacktrace:\n{}\nTerminating",
                ::vk::to_string(static_cast<::vk::Result>(r)),
                trace);
            std::terminate();
        }
    }
}

inline auto to_vk(ShaderStage s) -> ::vk::ShaderStageFlagBits
{
    using enum ShaderStage;
    switch (s)
    {
        case Vertex: return ::vk::ShaderStageFlagBits::eVertex;
        case Fragment: return ::vk::ShaderStageFlagBits::eFragment;
        case Compute: return ::vk::ShaderStageFlagBits::eCompute;
        default: throw arm::Exception("unknown shader stage");
    }
}

inline auto to_vk(Extent2D e) -> ::vk::Extent2D
{
    return {e.width, e.height};
}

inline auto to_vk(Offset2D o) -> ::vk::Offset2D
{
    return {o.x, o.y};
}

}
