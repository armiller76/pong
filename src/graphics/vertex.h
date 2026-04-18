#pragma once

#include "glm_wrapper.h"

namespace pong
{

struct Vertex
{
    ::glm::vec3 position;
    ::glm::vec4 color;
    ::glm::vec3 normal;
    ::glm::vec2 uv;
};

}
