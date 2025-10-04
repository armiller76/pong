#include "win32_window.h"

#include <string>
#include <windows.h>

#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

Win32Window::Win32Window(std::string_view app_name)
    : window_({})
    , running_(false)
{
    auto win32_instance = ::GetModuleHandleA(0);
    auto win32_window_class = WNDCLASS{};
    win32_window_class.lpfnWndProc = instance_window_callback;
    win32_window_class.hInstance = win32_instance;
    win32_window_class.lpszClassName = app_name.data();

    if (!::RegisterClassA(&win32_window_class))
    {
        throw arm::Exception("Failed to create window class");
    }

    window_ = {
        ::CreateWindowExA(
            WS_EX_OVERLAPPEDWINDOW,
            app_name.data(),
            app_name.data(),
            WS_OVERLAPPEDWINDOW,
            100,
            100,
            1200,
            780,
            0,
            0,
            win32_instance,
            this),
        ::DestroyWindow};

    ::ShowWindow(window_, SW_SHOWNORMAL);
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
    // get a pointer to the actual Win32Window instance so we can call into the actual event handler
    Win32Window *self = reinterpret_cast<Win32Window *>(::GetWindowLongPtrA(window, GWLP_USERDATA));

    // if the pointer exists, we already know where the window instance is and can call into its event handler
    if (self)
    {
        return self->handle_message(window, msg, wParam, lParam);
    }

    // if the pointer doesn't exist, and the message indicates a new window is being created, store the
    // pointer
    if (msg == WM_NCCREATE)
    {
        // windows will send us the window creation info in lParam, and we will have stored our pointer in its
        // lpParam when we called CreateWindowEx and passed it 'this'
        auto create_struct = reinterpret_cast<CREATESTRUCT *>(lParam);
        auto this_pointer = reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams);
        SetWindowLongPtr(window, GWLP_USERDATA, this_pointer);
        // TODO: windows docs indicate this should return TRUE maybe?
        return 0;
    }

    return DefWindowProcA(window, msg, wParam, lParam);
}

} // namespace pong
