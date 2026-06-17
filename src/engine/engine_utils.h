#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include <vulkan/vulkan_raii.hpp>

#include "graphics/color.h"
#include "math/rectangle.h"

namespace pong
{

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

enum class ResultCode
{
    Ok,
    Warning,
    Error,
    DescriptorSetLayoutCreateFailed,
};

constexpr auto to_string(ResultCode code)
{
    using enum ResultCode;
    switch (code)
    {
        case Ok: return "Ok";
        case Warning: return "Warning";
        case Error: return "Error";
        case DescriptorSetLayoutCreateFailed: return "DescriptorSetLayoutCreateFailed";

        default: return "<UNKNOWN>";
    }
}

struct EngineResult
{
    ResultCode code;
    std::string message;
};

} // namespace pong
