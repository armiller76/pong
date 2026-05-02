#pragma once

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

#include "graphics/glm_wrapper.h" // IWYU pragma: keep
#include "graphics/types.h"
#include "utils/exception.h"

// specializations so fastgltf can fill out glm vectors

template <>
struct ::fastgltf::ElementTraits<::glm::vec2>
    : ::fastgltf::ElementTraitsBase<::glm::vec2, ::fastgltf::AccessorType::Vec2, float>
{
};

template <>
struct ::fastgltf::ElementTraits<::glm::vec3>
    : ::fastgltf::ElementTraitsBase<::glm::vec3, ::fastgltf::AccessorType::Vec3, float>
{
};

template <>
struct ::fastgltf::ElementTraits<::glm::vec4>
    : ::fastgltf::ElementTraitsBase<::glm::vec4, ::fastgltf::AccessorType::Vec4, float>
{
};

namespace pong
{

constexpr auto inline to_pong(const ::fastgltf::math::fvec2 &v) -> ::glm::vec2
{
    return {v.x(), v.y()};
}

constexpr auto inline to_pong(const ::fastgltf::math::fvec3 &v) -> ::glm::vec3
{
    return {v.x(), v.y(), v.z()};
}

constexpr auto inline to_pong(const ::fastgltf::math::fvec4 &v) -> ::glm::vec4
{
    return {v.x(), v.y(), v.z(), v.w()};
}

constexpr auto inline to_pong(const ::fastgltf::math::quat<float> &q) -> ::glm::quat
{
    return {q.w(), q.x(), q.y(), q.z()};
}

constexpr auto inline to_pong(const ::fastgltf::math::fmat4x4 &m) -> ::glm::mat4x4
{
    return ::glm::mat4x4{to_pong(m[0]), to_pong(m[1]), to_pong(m[2]), to_pong(m[3])};
}

constexpr auto inline to_pong(::fastgltf::AlphaMode alpha_mode) -> AlphaMode
{
    switch (alpha_mode)
    {
        using enum ::fastgltf::AlphaMode;
        case Blend: return AlphaMode::Blend;
        case Opaque: return AlphaMode::Opaque;
        case Mask: return AlphaMode::Mask;
        default: throw arm::Exception("unknown AlphaMode");
    }
}

constexpr auto inline to_pong(::fastgltf::Wrap wrap) -> WrapMode
{
    switch (wrap)
    {
        using enum ::fastgltf::Wrap;
        case ClampToEdge: return WrapMode::ClampToEdge;
        case MirroredRepeat: return WrapMode::MirroredRepeat;
        case Repeat: return WrapMode::Repeat;
        default: throw arm::Exception("unknown WrapMode");
    }
}

} // namespace pong
