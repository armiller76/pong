#pragma once

#include <chrono>
#include <string>

#include <vulkan/vulkan_raii.hpp>

#include "engine/engine_types.h"
#include "engine/resource_loader.h"
#include "engine/resource_manager.h"
#include "engine/vulkan/vulkan_descriptor_pool.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_instance.h"
#include "engine/vulkan/vulkan_pipeline_manager.h"
#include "engine/vulkan/vulkan_renderer.h"
#include "engine/vulkan/vulkan_surface.h"
#include "imgui/imgui_wrapper.h"

namespace pong
{

using namespace std::literals;

class Camera;
class Scene;
class Win32Window;

class RenderContext
{
  public:
    explicit RenderContext(const RenderContextInfo &render_context_info, Win32Window &win32_window);

    auto load_scene(std::string_view filename) -> Scene;

    auto update_and_render(Scene &scene, Camera &camera) -> void; // or return a frame counter / diagnostic?

    auto shutdown() -> void;

  private:
    std::string app_name_;
    std::string engine_name_;
    [[maybe_unused]] Version version_;
    Win32Window &win32_window_;

    std::chrono::steady_clock::time_point last_window_recreate_time_;
    bool was_resize_pending_;

    ::vk::raii::Context vulkan_context_;

    VulkanInstance vulkan_instance_;
    VulkanSurface vulkan_surface_;
    VulkanDevice vulkan_device_;
    VulkanDescriptorPool vulkan_descriptor_pool_;
    ResourceManager resource_manager_;
    VulkanPipelineManager vulkan_pipeline_manager_;
    ResourceLoader resource_loader_;
    VulkanRenderer vulkan_renderer_;

    ImguiWrapper debug_renderer_;

  private:
    auto recreate_resources_() -> bool;
    auto init_() -> void;

    std::function<void(void)> debug_renderer_resize_callback_;
}; // class RenderContext

} // namespace pong
