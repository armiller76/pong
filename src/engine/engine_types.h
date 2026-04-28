#pragma once

#include <cstdint>
#include <string_view>

#include "graphics/color.h"
#include "math/rectangle.h"

namespace pong
{

using namespace std::literals;

struct Version
{
    std::uint32_t major;
    std::uint32_t minor;
    std::uint32_t patch;
};

struct RenderContextInfo
{
    std::string_view project_root;
    std::string_view app_name;
    std::string_view engine_name;
    std::uint32_t frames_in_flight;
    pong::Color clear_color;
    pong::Rectangle window_rect;
    Version version;
};

} // namespace pong
