#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>

#include <windows.h>

#include "engine/vulkan/vulkan_window.h"
#include "graphics/color.h"
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

class Win32Window : public VulkanWindow
{
  public:
    ~Win32Window() override;

    Win32Window(
        std::string_view application_name,
        Rectangle window_rect,
        Color clear_color = {0.42f, 0.42f, 0.42f, 1.0f});

    auto process_events() -> void override;
    auto handle_message(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;

    auto extent() const -> const Extent2D override;
    auto set_title(std::string_view title) -> void override;

    auto should_close() const -> bool override;
    auto win32_handles() const -> Win32WindowHandles;

    auto create_vulkan_surface(const VulkanInstance &instance) const -> VulkanSurface override;

    auto fire_close_callbacks() -> void override;
    auto add_close_callback(std::function<void()> close_callback) -> std::uint64_t override;
    auto remove_close_callback(std::uint64_t callback_id) -> void override;

    auto fire_resize_callbacks() -> void override;
    auto add_resize_callback(std::function<void(std::uint32_t, std::uint32_t)> resize_callback)
        -> std::uint64_t override;
    auto remove_resize_callback(std::uint64_t callback_id) -> void override;
    auto resize_pending() -> bool;
    auto is_minimized() -> bool;

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
