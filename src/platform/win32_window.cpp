#include "win32_window.h"

#include <string>
#include <windows.h>

#include "utils/error.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

Win32WindowCreateInfo::Win32WindowCreateInfo()
    : Win32WindowCreateInfo(0, 0, 800, 600)
{
    arm::log::warn("Win32WindowCreateInfo: No parameters, using defaults");
}

Win32WindowCreateInfo::Win32WindowCreateInfo(
    std::uint32_t x,
    std::uint32_t y,
    std::uint32_t width,
    std::uint32_t height)
    : x(x)
    , y(y)
    , width(width)
    , height(height)
{
    arm::ensure(x >= 0, "Win32WindowCreateInfo: Invalid x parameter ({})", x);
    arm::ensure(y >= 0, "Win32WindowCreateInfo: Invalid y parameter ({})", y);
    arm::ensure(width >= 0 && width <= 8192, "Win32WindowCreateInfo: Invalid width parameter ({})", width);
    arm::ensure(height >= 0 && height <= 4320, "Win32WindowCreateInfo: Invalid height parameter ({})", height);
}

Win32Window::Win32Window(std::string_view app_name, Win32WindowCreateInfo create_info)
    : window_({})
    , instance_(::GetModuleHandleA(0))
    , running_(false)
    , app_name_(app_name)
    , class_name_(std::string(app_name_ + "WindowClass"))
{
    auto win32_window_class = WNDCLASS{};
    win32_window_class.lpfnWndProc = instance_window_callback;
    win32_window_class.hInstance = instance_;
    win32_window_class.lpszClassName = class_name_.c_str();

    if (!::RegisterClassA(&win32_window_class))
    {
        throw arm::Exception("Failed to create window class");
    }

    window_ = {
        ::CreateWindowExA(
            WS_EX_OVERLAPPEDWINDOW,
            class_name_.c_str(),
            app_name_.c_str(),
            WS_OVERLAPPEDWINDOW,
            create_info.x,
            create_info.y,
            create_info.width,
            create_info.height,
            0,
            0,
            instance_,
            this),
        ::DestroyWindow};

    ::ShowWindow(window_, SW_SHOWNORMAL);
    running_ = true;
}

auto Win32Window::process_events() -> void
{
    ::MSG message;
    while (::PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE))
    {
        if (message.message == WM_QUIT)
        {
            running_ = false;
            break;
        }

        ::TranslateMessage(&message);
        ::DispatchMessageA(&message);
    }
}

auto Win32Window::handle_message(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    switch (msg)
    {
        case WM_CLOSE:
        {
            running_ = false;
            ::PostQuitMessage(0);
            break;
        }
        case WM_DESTROY:
        {
            ::PostQuitMessage(0);
            break;
        }
        default: break;
    }
    return ::DefWindowProcA(window, msg, wParam, lParam);
}

auto Win32Window::running() const -> bool
{
    return running_;
}

auto Win32Window::instance() const -> HINSTANCE
{
    return instance_;
}

auto Win32Window::handles() const -> WindowHandles
{
    return {window_, instance_};
}

auto CALLBACK Win32Window::instance_window_callback(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    Win32Window *self = reinterpret_cast<Win32Window *>(::GetWindowLongPtrA(window, GWLP_USERDATA));
    if (self)
    {
        return self->handle_message(window, msg, wParam, lParam);
    }

    if (msg == WM_NCCREATE)
    {
        auto create_struct = reinterpret_cast<CREATESTRUCT *>(lParam);
        auto this_pointer = reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams);
        SetWindowLongPtr(window, GWLP_USERDATA, this_pointer);
        return ::DefWindowProcA(window, msg, wParam, lParam);
    }

    return DefWindowProcA(window, msg, wParam, lParam);
}

} // namespace pong
