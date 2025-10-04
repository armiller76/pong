#pragma once
#include <string_view>

#include <windows.h>

#include "utils/auto_release.h"

namespace pong
{

class Win32Window
{
  public:
    Win32Window(std::string_view app_name);

    auto handle_message(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;

    auto running() const -> bool;

  private:
    static auto CALLBACK instance_window_callback(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;

    arm::AutoRelease<HWND, nullptr> window_;
    bool running_;
};

}
