#include "win32_window.h"

#include <map>
#include <string>
#include <windows.h>

#include "engine/engine_types.h"
#include "engine/vulkan/vulkan_instance.h"
#include "engine/vulkan/vulkan_surface.h"
#include "graphics/color.h"
#include "imgui.h"
#include "math/rectangle.h"
#include "utils/error.h"
#include "utils/exception.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace pong
{
using namespace std::literals;

Win32Window::~Win32Window()
{
    hwnd_.reset(0); // MUST be called here to ensure window is destroyed before instance handle is unregistered

    ::UnregisterClassA(class_name_.c_str(), hinstance_);
}

Win32Window::Win32Window(const RenderContextInfo &render_context_info)
    : hinstance_{::GetModuleHandleA(0)}
    , should_close_{false}
    , app_name_{render_context_info.app_name}
    , class_name_{std::string(app_name_ + "WindowClass")}
    , window_rect_{render_context_info.window_rect}
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
            window_rect_.extent.width,
            window_rect_.extent.height,
            0,
            0,
            hinstance_,
            this),
        ::DestroyWindow};

    if (!hwnd_)
    {
        throw arm::Exception("CreateWindowEx failed");
    }

    clear_brush_ = {
        ::CreateSolidBrush(
            RGB(Color::float_to_srgb_byte(render_context_info.clear_color.r),
                Color::float_to_srgb_byte(render_context_info.clear_color.g),
                Color::float_to_srgb_byte(render_context_info.clear_color.b))),
        ::DeleteObject};

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
        case WM_ERASEBKGND:
        {
            auto hdc = HDC(wParam);
            auto window_rect = RECT{};

            ::GetClientRect(hwnd_, &window_rect);
            ::FillRect(hdc, &window_rect, clear_brush_);
            return 1;
        }
        break;
        case WM_ENTERSIZEMOVE:
        {
            resize_pending_ = true;
            return ERROR_SUCCESS;
        }
        case WM_EXITSIZEMOVE:
        {
            resize_pending_ = false;
            return ERROR_SUCCESS;
        }
        case WM_SIZE:
        {
            if (wParam != SIZE_MINIMIZED)
            {
                is_minimized_ = false;
                window_rect_.extent.width = LOWORD(lParam);
                window_rect_.extent.height = HIWORD(lParam);
            }
            else
            {
                is_minimized_ = true;
            }

            return ERROR_SUCCESS;
        }

        case WM_CLOSE:
        {
            should_close_ = true;
            fire_close_callbacks();
            ::PostQuitMessage(0);
            return ERROR_SUCCESS;
        }

        case WM_DESTROY:
        {
            should_close_ = true;
            ::PostQuitMessage(0);
            return ERROR_SUCCESS;
        }

        default:
        {
            return ::DefWindowProcA(window, msg, wParam, lParam);
        }
    }
}

auto Win32Window::extent() const -> const Extent2D
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

auto Win32Window::fire_close_callbacks() -> void
{
    for (const auto &[id, callback] : close_callbacks_)
    {
        callback();
    }
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

auto Win32Window::fire_resize_callbacks() -> void
{
    for (const auto &[id, callback] : resize_callbacks_)
    {
        callback(window_rect_.extent.width, window_rect_.extent.height);
    }
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

auto Win32Window::resize_pending() -> bool
{
    return resize_pending_;
}

auto Win32Window::is_minimized() -> bool
{
    return is_minimized_;
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
