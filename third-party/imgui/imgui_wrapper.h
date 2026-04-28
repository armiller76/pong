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

class VulkanDescriptorPool;
class VulkanDevice;
class VulkanInstance;
class VulkanRenderer;

class ImguiWrapper

{
  public:
    ImguiWrapper(
        HWND hwnd,
        const VulkanInstance &instance,
        const VulkanDevice &device,
        VulkanRenderer &renderer,
        std::string_view project_root);
    ~ImguiWrapper();

    ImguiWrapper(const ImguiWrapper &) = delete;
    auto operator=(const ImguiWrapper &) -> ImguiWrapper & = delete;
    ImguiWrapper(ImguiWrapper &&) noexcept = delete;
    auto operator=(ImguiWrapper &&) noexcept -> ImguiWrapper & = delete;

    auto startup() -> void;
    auto init_windows() -> void;
    auto init_vulkan() -> void;
    auto shutdown() -> void;
    auto recreate() -> void;

    auto begin_frame() -> void;
    auto render() -> void;

    auto get_draw_data() -> ImDrawData *;
    auto process_window_event() -> void;

  private:
    ImGuiIO *io;
    HWND windows_handle_;
    const VulkanInstance &instance_;
    const VulkanDevice &device_;
    VulkanRenderer &renderer_;
    ImDrawData *draw_data_;
    std::string ini_file_;
};

}
