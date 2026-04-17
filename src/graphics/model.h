#pragma once

#include <string>
#include <vector>

#include "renderable.h"

namespace pong
{

struct Model
{
    std::string name;
    std::vector<Renderable> renderables;
};

} // namespace pong
