#pragma once
#include <cstdint>
#include <string>

#include <windows.h>

#include "utils/auto_release.h"

namespace pong
{

struct Win32WindowCreateInfo
{
    std::uint32_t x;
    std::uint32_t y;
    std::uint32_t width;
    std::uint32_t height;

    Win32WindowCreateInfo();

    Win32WindowCreateInfo(std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height);
};

struct WindowHandles
{
    const HWND window;
    const HINSTANCE instance;
};

class Win32Window
{
  public:
    Win32Window(std::string_view application_name, Win32WindowCreateInfo create_info);

    auto process_events() -> void;
    auto handle_message(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;

    auto running() const -> bool;
    auto instance() const -> HINSTANCE;
    auto handles() const -> WindowHandles;

  private:
    static auto CALLBACK instance_window_callback(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;

    arm::AutoRelease<HWND, static_cast<HWND>(0)> window_;
    HINSTANCE instance_;
    bool running_;
    std::string app_name_;
    std::string class_name_;
};

}
