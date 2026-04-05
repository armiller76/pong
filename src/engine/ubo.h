#pragma once

#include "graphics/glm_wrapper.h"

namespace pong
{

struct ubo_vp
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

}
