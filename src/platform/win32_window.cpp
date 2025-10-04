#include "win32_window.h"

#include <string>
#include <windows.h>

#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

Win32Window::Win32Window(std::string_view app_name, Win32WindowCreateInfo create_info)
    : window_({})
    , running_(false)
    , app_name_(app_name)
    , class_name_(std::string(app_name_ + "WindowClass"))
{
    auto win32_instance = ::GetModuleHandleA(0);
    auto win32_window_class = WNDCLASS{};
    win32_window_class.lpfnWndProc = instance_window_callback;
    win32_window_class.hInstance = win32_instance;
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
            win32_instance,
            this),
        ::DestroyWindow};

    ::ShowWindow(window_, SW_SHOWNORMAL);
    running_ = true;
}

auto Win32Window::handle_message(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    switch (msg)
    {
        case WM_CLOSE:
        {
            running_ = false;
        }
        default: break;
    }
    return ::DefWindowProcA(window, msg, wParam, lParam);
}

auto Win32Window::running() const -> bool
{
    return running_;
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
