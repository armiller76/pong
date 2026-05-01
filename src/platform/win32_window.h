#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>

#include <windows.h>

#include "engine/engine_types.h"
#include "math/rectangle.h"
#include "utils/auto_release.h"

namespace pong
{

struct Win32WindowHandles
{
    const HWND window;
    const HINSTANCE instance;
};

class VulkanInstance;
class VulkanSurface;

class Win32Window
{
  public:
    ~Win32Window();

    Win32Window(const RenderContextInfo &render_context_info);

    auto process_events() -> void;
    auto handle_message(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;

    auto extent() const -> const Extent2D;
    auto set_title(std::string_view title) -> void;

    auto should_close() const -> bool;
    auto win32_handles() const -> const Win32WindowHandles;

    auto fire_close_callbacks() const -> void;
    auto add_close_callback(std::function<void()> close_callback) -> std::uint64_t;
    auto remove_close_callback(std::uint64_t callback_id) -> void;

    auto fire_resize_callbacks() const -> void;
    auto add_resize_callback(std::function<void(std::uint32_t, std::uint32_t)> resize_callback) -> std::uint64_t;
    auto remove_resize_callback(std::uint64_t callback_id) -> void;

    auto resize_pending() const -> bool;
    auto is_minimized() const -> bool;

  private:
    HINSTANCE hinstance_;

    bool should_close_ = false;
    bool resize_pending_ = false;
    bool is_minimized_ = false;

    std::string app_name_;
    std::string class_name_;
    Rectangle window_rect_;
    arm::AutoRelease<HBRUSH, static_cast<HBRUSH>(0)> clear_brush_;
    arm::AutoRelease<HWND, static_cast<HWND>(0)> hwnd_;

    std::map<std::uint64_t, std::function<void()>> close_callbacks_{};
    std::map<std::uint64_t, std::function<void(std::uint32_t, std::uint32_t)>> resize_callbacks_{};
    std::uint64_t current_callback_token_{};

    static auto CALLBACK instance_window_callback(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;
};

}
