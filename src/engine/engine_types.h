#pragma once

#include <cstdint>
#include <string>

namespace pong
{

enum class ResultCode : std::uint8_t
{
    Ok,
    Warning,
    Error,
    DescriptorSetLayoutCreateFailed,
};

struct EngineResult
{
    ResultCode code;
    std::string message;
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

} // namespace pong
