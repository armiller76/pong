#pragma once

#include <string>

#include <vulkan/vulkan.hpp>

#include "utils/exception.h"

namespace pong
{

enum class AlphaMode
{
    Opaque,
    Mask,
    Blend,
};

enum class FilterMode
{
    Nearest,
    Linear,
};

enum class MipMapMode
{
    Nearest,
    Linear,
};

enum class WrapMode
{
    Repeat,
    MirroredRepeat,
    ClampToEdge,
};

enum class CompareOp
{
    Never,
    Equal,
    Less,
    LessOrEqual,
    Greater,
    GreaterOrEqual,
    Always,
};

constexpr auto to_string(AlphaMode alpha_mode) -> std::string
{
    switch (alpha_mode)
    {
        case AlphaMode::Blend: return "blend";
        case AlphaMode::Mask: return "mask";
        case AlphaMode::Opaque: return "opaque";
    }
}

constexpr auto to_vk(FilterMode m) -> ::vk::Filter
{
    switch (m)
    {
        case FilterMode::Linear: return ::vk::Filter::eLinear;
        case FilterMode::Nearest: return ::vk::Filter::eNearest;
        default: throw arm::Exception("invalid filter mode");
    }
}

constexpr auto to_vk(MipMapMode m) -> ::vk::SamplerMipmapMode
{
    switch (m)
    {
        case MipMapMode::Linear: return ::vk::SamplerMipmapMode::eLinear;
        case MipMapMode::Nearest: return ::vk::SamplerMipmapMode::eNearest;
        default: throw arm::Exception("invalid filter mode");
    }
}

constexpr auto to_vk(WrapMode m) -> ::vk::SamplerAddressMode
{
    switch (m)
    {
        case WrapMode::ClampToEdge: return ::vk::SamplerAddressMode::eClampToEdge;
        case WrapMode::Repeat: return ::vk::SamplerAddressMode::eRepeat;
        case WrapMode::MirroredRepeat: return ::vk::SamplerAddressMode::eMirroredRepeat;
        default: throw arm::Exception("invalid sampler wrap/address mode");
    }
}

constexpr auto to_vk(CompareOp o) -> ::vk::CompareOp
{
    switch (o)
    {
        case CompareOp::Never: return ::vk::CompareOp::eNever;
        case CompareOp::Always: return ::vk::CompareOp::eAlways;
        case CompareOp::Equal: return ::vk::CompareOp::eEqual;
        case CompareOp::Less: return ::vk::CompareOp::eLess;
        case CompareOp::LessOrEqual: return ::vk::CompareOp::eLessOrEqual;
        case CompareOp::Greater: return ::vk::CompareOp::eGreater;
        case CompareOp::GreaterOrEqual: return ::vk::CompareOp::eGreaterOrEqual;
        default: throw arm::Exception("invalid compare op");
    }
}

} // namespace pong
