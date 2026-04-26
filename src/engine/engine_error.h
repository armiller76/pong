#pragma once

#include <expected>
#include <format>
#include <string>

#include <vulkan/vulkan_raii.hpp>

namespace pong
{

enum class ResultCode
{
    Ok,
    Warning,
    Error,
};

struct EngineResult
{
    ResultCode code;
    std::string message;
};

[[nodiscard]] static inline auto check_vk_result(VkResult r) -> EngineResult
{
    auto result = static_cast<::vk::Result>(r);
    if (result == ::vk::Result::eSuccess)
    {
        return {ResultCode::Ok, {}};
    }
    else
    {
        return {ResultCode::Error, std::format("Vulkan result: {}", ::vk::to_string(result))};
    }
}

template <typename T>
[[nodiscard]] static inline auto check_vk_expected(std::expected<T, ::vk::Result> &&check)
    -> std::expected<T, EngineResult>
{

    if (!check) // error state
    {
        switch (check.error())
        {
            using enum ::vk::Result;
            case eSuccess: return {ResultCode::Ok, {}};
            default: return {ResultCode::Error, std::format("Vulkan result: {}", check.error())};
        }
    }
    else // success state
    {
        return std::move(check.value());
    }
}

} // namespace pong
