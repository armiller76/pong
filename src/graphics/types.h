#pragma once

#include <string>

namespace pong
{

enum class AlphaMode
{
    Opaque,
    Mask,
    Blend,
};

enum class MagFilterMode
{
    Nearest,
    Linear,
};

enum class MinFilterMode
{
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear,
};

enum class WrapMode
{
    Repeat,
    MirroredRepeat,
    ClampToEdge,
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

} // namespace pong
