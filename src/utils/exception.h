#pragma once

#include <cstdint>
#include <format>
#include <stacktrace>
#include <string>

namespace arm
{

class Exception
{
  public:
    // Construct a new Exception. Captures the current stack trace.
    template <class... Args>
    constexpr Exception(std::format_string<Args...> what, Args &&...args)
        : trace_(std::stacktrace::current(1))
        , what_(std::format(what, std::forward<Args>(args)...))
    {
    }

    // Construct a new Exception. Captures the current stack trace but allows to skip a number of frames.
    template <class... Args>
    constexpr Exception(std::format_string<Args...> what, Args &&...args, std::uint32_t skip)
        : trace_(std::stacktrace::current(skip))
        , what_(std::format(what, std::forward<Args>(args)...))
    {
    }

    auto stacktrace() const -> std::string;
    auto what() const -> std::string;
    auto to_string() const -> std::string;

  private:
    std::stacktrace trace_;
    std::string what_;
};

}
