#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <windows.h>

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_win32.h"

#include "engine/vulkan/vulkan_utils.h"
#include "utils/error.h"

namespace pong
{

class VulkanDevice;
class VulkanInstance;

class ImguiWrapper

{
  public:
    ImguiWrapper(HWND hwnd, VulkanInstance &instance, VulkanDevice &device);

    auto initialize() -> void;
    auto shutdown() -> void;

    auto begin_frame() -> void;
    auto render() -> void;
    auto end_frame() -> void;

    auto swapchain_recreated_callback() -> void;
    auto process_window_event() -> void;

  private:
    ImGuiIO io;
    HWND windows_handle_;
    VulkanInstance &instance_;
    VulkanDevice &device_;
};

}

/*
A thin ImGui wrapper is the right move here, especially with your dynamic-rendering renderer.

Start with one focused class that does only lifecycle and frame bridging, not app UI logic.

Suggested wrapper responsibilities

Initialize
Create ImGui context.
Initialize Win32 backend with HWND from win32_window.h.
Initialize Vulkan backend using your existing Vulkan objects and dynamic rendering settings.
Use swapchain format from vulkan_swapchain.h and queue/device from vulkan_device.h.
Per-frame start
Call NewFrame for Vulkan backend, Win32 backend, and Dear ImGui.
Expose a method like begin_frame so game code can build widgets.
Per-frame render
Call ImGui::Render and ImGui_ImplVulkan_RenderDrawData with current command buffer.
This should happen inside your existing dynamic rendering region between beginRendering and endRendering in
vulkan_renderer.cpp:150 and vulkan_renderer.cpp:205. Resize/swapchain updates On swapchain recreate, notify ImGui
backend of min image count changes. Keep this hook near your recreate path in vulkan_renderer.cpp:86. Shutdown Shutdown
Vulkan backend, Win32 backend, then destroy context. Best insertion points in your current flow

Event forwarding
Forward Win32 messages to ImGui first inside win32_window.cpp:88 (or in the static callback path at
win32_window.cpp:192). If ImGui consumes the message, return immediately. Main loop Build UI each frame in loop around
pong.cpp:74, pong.cpp:82, pong.cpp:84. Sequence: process events, begin imgui frame, define widgets, renderer render.
Renderer integration
Either:
pass wrapper into renderer and let renderer call its render_draw_data during record, or
have renderer accept a callback invoked before endRendering.
Your render entry point is vulkan_renderer.cpp:79.
Dynamic rendering specifics you must set

UseDynamicRendering true in ImGui Vulkan init info.
Fill pipeline rendering create info with your swapchain color format.
Keep color attachment format in ImGui init matched with what renderer uses for vulkan_renderer.cpp:121.
Ensure descriptor pool capacity is sufficient for ImGui textures.
Two design choices that keep this clean

Wrapper owns only ImGui backend state and an internal descriptor pool.
App owns UI content via a callback, for example build_ui(fn), so gameplay code stays out of backend glue.
If you want, I can draft a concrete class API next that matches your current types exactly (Win32Window, VulkanDevice,
VulkanRenderer) and keeps changes to three touchpoints only.*/
