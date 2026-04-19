#pragma once

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

}
