#pragma once

#include <string>
#include <string_view>
#include <vulkan/vulkan_raii.hpp>
#include <windows.h>

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_win32.h"

#include "engine/vulkan/vulkan_utils.h"
#include "utils/error.h"

namespace pong
{

class VulkanDescriptorPool;
class VulkanDevice;
class VulkanInstance;
class VulkanRenderer;

class ImguiWrapper

{
  public:
    ImguiWrapper(HWND hwnd, VulkanRenderer &renderer, const VulkanInstance &instance, std::string_view project_root);
    ~ImguiWrapper();

    ImguiWrapper(const ImguiWrapper &) = delete;
    auto operator=(const ImguiWrapper &) -> ImguiWrapper & = delete;
    ImguiWrapper(ImguiWrapper &&) noexcept = delete;
    auto operator=(ImguiWrapper &&) noexcept -> ImguiWrapper & = delete;

    auto startup() -> void;
    auto init_windows() -> void;
    auto init_vulkan() -> void;
    auto shutdown() -> void;
    void framebuffer_resize_callback();

    auto begin_frame() -> void;
    auto render() -> void;

    auto get_draw_data() -> ImDrawData *;
    auto process_window_event() -> void;

  private:
    ImGuiIO *io;
    HWND windows_handle_;
    VulkanRenderer &vk_renderer_;
    const VulkanInstance &vk_instance_;
    ImDrawData *draw_data_;
    std::string ini_file_;
};

}
