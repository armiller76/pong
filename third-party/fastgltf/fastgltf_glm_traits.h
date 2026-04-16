#pragma once

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

#include "graphics/glm_wrapper.h"

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
