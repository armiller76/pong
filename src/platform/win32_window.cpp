#include "win32_window.h"

#include <algorithm>
#include <climits>
#include <cstddef>
#include <string>
#include <vector>

#include <hidusage.h>
#include <windowsx.h>

#include "imgui.h"

#include "core/key.h"
#include "engine/engine_types.h"
#include "engine/input_state.h"
#include "event/key_event.h"
#include "event/mouse_button_event.h"
#include "event/mouse_move_event.h"
#include "graphics/color.h"
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

Win32Window::Win32Window(const RenderContextInfo &render_context_info, InputState &input_state)
    : hinstance_{::GetModuleHandleA(0)}
    , input_state{input_state}
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

    const auto mouse_info = ::RAWINPUTDEVICE{
        .usUsagePage = HID_USAGE_PAGE_GENERIC,
        .usUsage = HID_USAGE_GENERIC_MOUSE,
        .dwFlags = {}, // maybe add RIDEV_INPUTSINK for background input?
        .hwndTarget = hwnd_,
    };

    arm::ensure(::RegisterRawInputDevices(&mouse_info, 1u, sizeof(mouse_info)), "failed to register for mouse input");

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

    // TODO handle focus loss (notify InputState to clear all key-down or button-down status)
    switch (msg)
    {
        case WM_INPUT:
        {
            const auto input_code = GET_RAWINPUT_CODE_WPARAM(wParam);
            if (input_code == RIM_INPUT)
            {
                auto size = ::UINT{};
                ::GetRawInputData(
                    reinterpret_cast<::HRAWINPUT>(lParam), RID_INPUT, nullptr, &size, sizeof(::RAWINPUTHEADER));
                auto buffer = std::vector<std::byte>(size);
                ::GetRawInputData(
                    reinterpret_cast<::HRAWINPUT>(lParam), RID_INPUT, buffer.data(), &size, sizeof(::RAWINPUTHEADER));
                const auto raw = reinterpret_cast<const ::RAWINPUT *>(buffer.data());

                if (raw->header.dwType == RIM_TYPEMOUSE)
                {
                    auto &mouse_data = raw->data.mouse;

                    if (mouse_data.usFlags & MOUSE_MOVE_ABSOLUTE)
                    {
                        auto bounds = ::RECT{};
                        if (mouse_data.usFlags & MOUSE_VIRTUAL_DESKTOP)
                        {
                            bounds.left = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
                            bounds.top = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
                            bounds.right = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
                            bounds.bottom = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
                        }
                        else
                        {
                            bounds.left = 0l;
                            bounds.top = 0l;
                            bounds.right = ::GetSystemMetrics(SM_CXSCREEN);
                            bounds.bottom = ::GetSystemMetrics(SM_CYSCREEN);
                        }
                        const auto x =
                            static_cast<float>(last_mouse_x_ - ::MulDiv(mouse_data.lLastX, bounds.right, USHRT_MAX));
                        const auto y =
                            static_cast<float>(last_mouse_y_ - ::MulDiv(mouse_data.lLastY, bounds.bottom, USHRT_MAX));
                        input_state.events().emplace(MouseMoveEvent{x, y});
                    }
                    else if (mouse_data.lLastX != 0 || mouse_data.lLastY != 0)
                    {
                        const auto x = static_cast<float>(mouse_data.lLastX);
                        const auto y = static_cast<float>(mouse_data.lLastY);

                        input_state.events().emplace(MouseMoveEvent{x, y});
                    }
                }
                ::DefWindowProcA(window, msg, wParam, lParam);
                return 0;
            }
            else
            {
                // application not focused, ignore inputs
                return 0;
            }
        }
        break;

        case WM_LBUTTONDOWN:
        {
            input_state.events().emplace(MouseButtonEvent{MouseButton::Left, MouseButtonState::Down});
            return 0;
        }
        break;

        case WM_LBUTTONUP:
        {
            input_state.events().emplace(MouseButtonEvent{MouseButton::Left, MouseButtonState::Up});
            return 0;
        }
        break;

        case WM_MBUTTONDOWN:
        {
            input_state.events().emplace(MouseButtonEvent{MouseButton::Middle, MouseButtonState::Down});
            return 0;
        }
        break;

        case WM_MBUTTONUP:
        {
            input_state.events().emplace(MouseButtonEvent{MouseButton::Middle, MouseButtonState::Up});
            return 0;
        }
        break;

        case WM_RBUTTONDOWN:
        {
            input_state.events().emplace(MouseButtonEvent{MouseButton::Right, MouseButtonState::Down});
            return 0;
        }
        break;

        case WM_RBUTTONUP:
        {
            input_state.events().emplace(MouseButtonEvent{MouseButton::Right, MouseButtonState::Up});
            return 0;
        }
        break;

        case WM_KEYDOWN:
        {
            auto key = static_cast<Key>(wParam);
            if (std::ranges::contains(all_keys, key))
            {
                input_state.events().emplace(KeyEvent{key, KeyPosition::Down});
                input_state.dirty_keys().insert(key);
                return 0;
            }
            else
            {
                return DefWindowProcA(hwnd_, msg, lParam, wParam);
            }
        }

        case WM_KEYUP:
        {
            auto key = static_cast<Key>(wParam);
            if (std::ranges::contains(all_keys, key))
            {
                input_state.events().emplace(KeyEvent{key, KeyPosition::Up});
                input_state.dirty_keys().insert(key);
                return 0;
            }
            else
            {
                return DefWindowProcA(hwnd_, msg, lParam, wParam);
            }
        }

        case WM_ERASEBKGND:
        {
            auto hdc = HDC(wParam);
            auto window_rect = RECT{};

            ::GetClientRect(hwnd_, &window_rect);
            ::FillRect(hdc, &window_rect, clear_brush_);
            return 1;
        }

        case WM_ENTERSIZEMOVE:
        {
            resize_pending_ = true;
            return 0;
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

            return 0;
        }

        case WM_EXITSIZEMOVE:
        {
            resize_pending_ = false;
            return 0;
        }

        case WM_CLOSE:
        {
            should_close_ = true;
            fire_close_callbacks();
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

auto Win32Window::win32_handles() const -> const Win32WindowHandles
{
    return {hwnd_, hinstance_};
}

auto Win32Window::fire_close_callbacks() const -> void
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

auto Win32Window::fire_resize_callbacks() const -> void
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

auto Win32Window::resize_pending() const -> bool
{
    return resize_pending_;
}

auto Win32Window::is_minimized() const -> bool
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
