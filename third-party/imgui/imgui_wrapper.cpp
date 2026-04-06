#include "imgui_wrapper.h"

#include <vulkan/vulkan_raii.hpp>
#include <windows.h>

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_win32.h"

#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_instance.h"
#include "engine/vulkan/vulkan_utils.h"
#include "utils/error.h"

namespace pong
{

ImguiWrapper::ImguiWrapper(HWND hwnd, VulkanInstance &instance, VulkanDevice &device)
    : io{[]()
         {
             IMGUI_CHECKVERSION();
             ImGui::CreateContext();
             return ImGui::GetIO();
         }()}
    , windows_handle_{hwnd}
    , instance_{instance}
    , device_{device}
{
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui_ImplWin32_Init(windows_handle_);

    auto init_info = ImGui_ImplVulkan_InitInfo{};
    init_info.Instance = *instance_.get();
    init_info.PhysicalDevice = *device_.physical_device();
    init_info.Device = *device_.native_handle();
    // TODO LEFT OFF HERE
    ImGui_ImplVulkan_Init(&init_info);
}

}
