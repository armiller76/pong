#pragma once

#include <expected>
#include <format>
#include <string>
#include <utility>

#include <vulkan/vulkan_raii.hpp>

#include "utils/exception.h"

namespace pong
{

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

[[maybe_unused]] [[nodiscard]] static inline auto check_vk_result(::vk::Result r) -> EngineResult
{
    if (r == ::vk::Result::eSuccess)
    {
        return {ResultCode::Ok, {}};
    }
    else
    {
        return {ResultCode::Error, std::format("Vulkan result: {}", ::vk::to_string(r))};
    }
}

[[maybe_unused]] static inline auto check_vk_result(VkResult err) -> void
{
    if (err == 0)
    {
        return;
    }
    else
    {
        throw arm::Exception("Vulkan error: {}", vk::to_string(static_cast<::vk::Result>(err)));
    }
}

template <typename T>
[[nodiscard]] static inline auto check_vk_expected(std::expected<T, ::vk::Result> &&check)
    -> std::expected<T, EngineResult>
{
    if (!check) // error state
    {
        return std::unexpected(EngineResult{ResultCode::Error, ::vk::to_string(check.error())});
    }
    else // success state
    {
        return std::move(check.value());
    }
}

} // namespace pong
