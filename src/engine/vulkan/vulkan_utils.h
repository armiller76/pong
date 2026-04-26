#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "graphics/image_format.h"
#include "graphics/shader.h"
#include "math/rectangle.h"
#include "utils/exception.h"

namespace pong
{

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

inline auto to_vk(ImageFormat f) -> ::vk::Format
{
    // TODO consider running this list thru device selection? or a subset?
    using enum ImageFormat;
    using enum ::vk::Format;
    switch (f)
    {
        case RGBA8: return eR8G8B8A8Unorm;
        case RGB8: return eR8G8B8Unorm;
        case BGRA8: return eB8G8R8A8Unorm;
        case R8: return eR8Unorm;
        case RG8: return eR8G8Unorm;
        case RGBA16F: return eR16G16B16A16Sfloat;
        case RGBA32F: return eR32G32B32A32Sfloat;
        case RGBA8_sRGB: return eR8G8B8A8Srgb;
        case D16: return eD16Unorm;
        case D24: return eX8D24UnormPack32;
        case D32F: return eD32Sfloat;
        case D24S8: return eD24UnormS8Uint;
        case D32FS8: return eD32SfloatS8Uint;
        default: return eUndefined;
    }
}

inline auto to_pong(::vk::Format f) -> ImageFormat
{
    using enum ::vk::Format;
    using enum ImageFormat;
    switch (f)
    {
        case eR8G8B8A8Unorm: return RGBA8;
        case eR8G8B8Unorm: return RGB8;
        case eB8G8R8A8Unorm: return BGRA8;
        case eR8Unorm: return R8;
        case eR8G8Unorm: return RG8;
        case eR16G16B16A16Sfloat: return RGBA16F;
        case eR32G32B32A32Sfloat: return RGBA32F;
        case eR8G8B8A8Srgb: return RGBA8_sRGB;
        case eD16Unorm: return D16;
        case eD32Sfloat: return D32F;
        case eX8D24UnormPack32: return D24;
        case eD24UnormS8Uint: return D24S8;
        case eD32SfloatS8Uint: return D32FS8;
        default: return UNDEFINED;
    }
}

}
