#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <string_view>

#include <windows.h>

#include "math/rectangle.h"
#include "utils/auto_release.h"

namespace pong
{

struct Win32WindowHandles
{
    const HWND window;
    const HINSTANCE instance;
};

class InputState;
class VulkanInstance;
class VulkanSurface;
struct RenderContextInfo;

class Win32Window
{
  public:
    ~Win32Window();

    Win32Window(std::string_view app_name, const RenderContextInfo &render_context_info, InputState &input_state);

    auto process_events() -> void;
    auto handle_message(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;

    [[nodiscard]] auto extent() const -> const Extent2D;
    auto set_title(std::string_view title) -> void;

    [[nodiscard]] auto should_close() const -> bool;
    [[nodiscard]] auto win32_handles() const -> const Win32WindowHandles;

    auto fire_close_callbacks() const -> void;
    [[nodiscard]] auto add_close_callback(std::function<void()> close_callback) -> std::uint64_t;
    auto remove_close_callback(std::uint64_t callback_id) -> void;

    auto fire_resize_callbacks() const -> void;
    [[nodiscard]] auto add_resize_callback(std::function<void(std::uint32_t, std::uint32_t)> resize_callback)
        -> std::uint64_t;
    auto remove_resize_callback(std::uint64_t callback_id) -> void;

    [[nodiscard]] auto resize_pending() const -> bool;
    [[nodiscard]] auto is_minimized() const -> bool;

  private:
    HINSTANCE hinstance_;
    InputState &input_state;
    std::int32_t last_mouse_x_;
    std::int32_t last_mouse_y_;

    bool should_close_ = false;
    bool resize_pending_ = false;
    bool is_minimized_ = false;

    std::string app_name_;
    std::string class_name_;
    Rectangle window_rect_;
    arm::AutoRelease<HBRUSH, static_cast<HBRUSH>(nullptr)> clear_brush_;
    arm::AutoRelease<HWND, static_cast<HWND>(nullptr)> hwnd_;

    std::map<std::uint64_t, std::function<void()>> close_callbacks_{};
    std::map<std::uint64_t, std::function<void(std::uint32_t, std::uint32_t)>> resize_callbacks_{};
    std::uint64_t current_callback_token_{};

    static auto CALLBACK instance_window_callback(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;
};

}
