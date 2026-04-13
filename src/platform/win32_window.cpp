#include "win32_window.h"

#include <map>
#include <string>
#include <windows.h>

#include "engine/vulkan/vulkan_instance.h"
#include "engine/vulkan/vulkan_surface.h"
#include "imgui.h"
#include "math/rectangle.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/log.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace pong
{

Win32Window::~Win32Window()
{
    ::UnregisterClassA(class_name_.c_str(), hinstance_);
}

Win32Window::Win32Window(std::string_view app_name, Rectangle window_rect)
    : hinstance_{::GetModuleHandleA(0)}
    , should_close_{false}
    , app_name_{app_name}
    , class_name_{std::move(std::string(app_name_ + "WindowClass"))}
    , window_rect_{window_rect}
{
    arm::ensure(
        (window_rect_.extent.width != 0) && (window_rect_.extent.height != 0),
        "Invalid window dimension: w={} h={}",
        window_rect_.extent.width,
        window_rect_.extent.height);

    // TODO: Consider some parameterization here?
    auto win32_window_class = WNDCLASS{};
    win32_window_class.lpfnWndProc = instance_window_callback;
    win32_window_class.hInstance = hinstance_;
    win32_window_class.lpszClassName = class_name_.c_str();

    if (!::RegisterClassA(&win32_window_class))
    {
        throw arm::Exception("Failed to register window class");
    }

    hwnd_ = {
        ::CreateWindowExA(
            WS_EX_OVERLAPPEDWINDOW,
            class_name_.c_str(),
            app_name_.c_str(),
            WS_OVERLAPPEDWINDOW,
            window_rect_.offset.x,
            window_rect_.offset.y,
            window_rect.extent.width,
            window_rect.extent.height,
            0,
            0,
            hinstance_,
            this),
        ::DestroyWindow};

    if (!hwnd_)
    {
        throw arm::Exception("CreateWindowEx failed");
    }

    ::ShowWindow(hwnd_, SW_SHOWNORMAL);
    ::UpdateWindow(hwnd_);
}

auto Win32Window::process_events() -> void
{
    ::MSG message;
    while (::PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE))
    {
        if (message.message == WM_QUIT)
        {
            should_close_ = true;
            break;
        }

        ::TranslateMessage(&message);
        ::DispatchMessageA(&message);
    }
}

auto Win32Window::handle_message(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    if (ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam))
    {
        return true;
    }

    switch (msg)
    {
        case WM_SIZE:
        {
            if (wParam != SIZE_MINIMIZED)
            {
                window_rect_.extent.width = LOWORD(lParam);
                window_rect_.extent.height = HIWORD(lParam);

                for (const auto &[id, callback] : resize_callbacks_)
                {
                    callback(window_rect_.extent.width, window_rect_.extent.height);
                }
            }
            return 0;
        }

        case WM_CLOSE:
        {
            should_close_ = true;

            for (const auto &[id, callback] : close_callbacks_)
            {
                callback();
            }

            ::PostQuitMessage(0);
            return 0;
        }

        case WM_DESTROY:
        {
            should_close_ = true;
            ::PostQuitMessage(0);
            return 0;
        }

        default:
        {
            return ::DefWindowProcA(window, msg, wParam, lParam);
        }
    }
}

auto Win32Window::extent() const -> Extent2D
{
    return window_rect_.extent;
}

auto Win32Window::set_title(std::string_view title) -> void
{
    ::SetWindowTextA(hwnd_, std::string(title).c_str());
}

auto Win32Window::should_close() const -> bool
{
    return should_close_;
}

auto Win32Window::win32_handles() const -> Win32WindowHandles
{
    return {hwnd_, hinstance_};
}

auto Win32Window::create_vulkan_surface(const VulkanInstance &instance) const -> VulkanSurface
{
    return VulkanSurface{instance, win32_handles()};
}

auto Win32Window::add_close_callback(std::function<void()> close_callback) -> std::uint64_t
{
    auto token = ++current_callback_token_;
    close_callbacks_.emplace(token, close_callback);
    return token;
}

auto Win32Window::remove_close_callback(std::uint64_t callback_handle) -> void
{
    arm::ensure(
        close_callbacks_.contains(callback_handle),
        "Can't remove close_callbacks_ handle ({}) which is not in the map",
        callback_handle);
    close_callbacks_.erase(callback_handle);
}

auto Win32Window::add_resize_callback(std::function<void(std::uint32_t, std::uint32_t)> resize_callback)
    -> std::uint64_t
{
    auto token = ++current_callback_token_;
    resize_callbacks_.emplace(token, resize_callback);
    return token;
}

auto Win32Window::remove_resize_callback(std::uint64_t callback_handle) -> void
{
    arm::ensure(
        resize_callbacks_.contains(callback_handle),
        "Can't remove resize_callbacks_ handle ({}) which is not in the map",
        callback_handle);
    resize_callbacks_.erase(callback_handle);
}

auto CALLBACK Win32Window::instance_window_callback(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    if (msg == WM_NCCREATE)
    {
        auto create_struct = reinterpret_cast<CREATESTRUCT *>(lParam);
        auto this_pointer = reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams);
        SetWindowLongPtr(window, GWLP_USERDATA, this_pointer);
        return TRUE;
    }

    Win32Window *self = reinterpret_cast<Win32Window *>(::GetWindowLongPtrA(window, GWLP_USERDATA));
    if (self)
    {
        return self->handle_message(window, msg, wParam, lParam);
    }

    return DefWindowProcA(window, msg, wParam, lParam);
}

} // namespace pong
