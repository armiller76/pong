#pragma once

#include <string>
#include <string_view>
#include <vulkan/vulkan_raii.hpp>
#include <windows.h>

#include "imgui.h"             // IWYU pragma: keep
#include "imgui_impl_vulkan.h" // IWYU pragma: keep
#include "imgui_impl_win32.h"  // IWYU pragma: keep

namespace pong
{

class VulkanInstance;
class RenderContext;

class ImguiWrapper

{
  public:
    ImguiWrapper(
        HWND hwnd,
        const VulkanInstance &instance,
        RenderContext &render_context,
        std::string_view project_root);
    ~ImguiWrapper();

    ImguiWrapper(const ImguiWrapper &) = delete;
    auto operator=(const ImguiWrapper &) -> ImguiWrapper & = delete;
    ImguiWrapper(ImguiWrapper &&) noexcept = delete;
    auto operator=(ImguiWrapper &&) noexcept -> ImguiWrapper & = delete;

    auto draw_fps() -> void;

    auto draw_left_panel() -> void;

    auto draw_settings() -> void;

    auto render() -> void;

    auto recreate() -> void;

    auto get_draw_data() -> ImDrawData *;

    auto shutdown() -> void;

  private:
    ImGuiIO *io_;
    ImDrawData *draw_data_;

    HWND windows_handle_;
    const VulkanInstance &instance_;
    RenderContext &render_context_;
    std::string ini_file_;

  private:
    auto startup_() -> void;
    auto init_windows_() -> void;
    auto init_vulkan_() -> void;
};

}
