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
            throw arm::Exception("Vulkan fatal error: {}Terminating", ::vk::to_string(static_cast<::vk::Result>(r)));
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
        default: throw arm::Exception("unknown internal shader stage");
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

inline auto to_pong(::vk::ShaderStageFlagBits b) -> ShaderStage
{
    if (b & ::vk::ShaderStageFlagBits::eVertex)
        return ShaderStage::Vertex;
    if (b & ::vk::ShaderStageFlagBits::eFragment)
        return ShaderStage::Fragment;
    if (b & ::vk::ShaderStageFlagBits::eCompute)
        return ShaderStage::Compute;
    throw arm::Exception("unknown Vulkan ShaderStageFlag");
}

inline auto to_pong(::vk::Extent2D e) -> Extent2D
{
    return {e.width, e.height};
}

inline auto to_pong(::vk::Offset2D o) -> Offset2D
{
    return {o.x, o.y};
}

}
