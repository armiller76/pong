// log.h
// shamelessly stolen from @nathan_baggs

#pragma once

#include <format>
#include <print>
#include <string_view>

#include "formatter.h" // IWYU pragma: keep

namespace arm::log
{

namespace detail
{

template <char C, class... Args>
auto print(std::string_view fmt, Args &&...args) -> void
{
    std::println("[{}] {}", C, std::vformat(fmt, std::make_format_args(args...)));
}

} // namespace detail

struct debug
{
    template <class... Args>
    debug(std::string_view fmt, Args &&...args)
    {
        detail::print<'D'>(fmt, std::forward<Args>(args)...);
    }
};

struct info
{
    template <class... Args>
    info(std::string_view fmt, Args &&...args)
    {
        detail::print<'I'>(fmt, std::forward<Args>(args)...);
    }
};

struct warn
{
    template <class... Args>
    warn(std::string_view fmt, Args &&...args)
    {
        detail::print<'W'>(fmt, std::forward<Args>(args)...);
    }
};

struct error
{
    template <class... Args>
    error(std::string_view fmt, Args &&...args)
    {
        detail::print<'E'>(fmt, std::forward<Args>(args)...);
    }
};

} // namespace arm::log
