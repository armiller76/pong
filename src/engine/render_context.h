#pragma once

#include <chrono>
#include <cstdint>

#include "engine/resource_loader.h"
#include "engine/resource_manager.h"
#include "engine/vulkan/vulkan_descriptor_pool.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_pipeline_manager.h"
#include "engine/vulkan/vulkan_renderer.h"
#include "engine/vulkan/vulkan_surface.h"
#include "graphics/color.h"
#include "math/rectangle.h"

namespace pong
{

using namespace std::literals;

class Camera;
class InputState;
class ImguiWrapper;
class Scene;
class VulkanDevice;
class VulkanInstance;
class Win32Window;

struct RenderContextInfo
{
    std::uint32_t frames_in_flight;
    Color clear_color;
    Rectangle window_rect;
};

class RenderContext
{
  public:
    explicit RenderContext(
        const RenderContextInfo &render_context_info,
        Win32Window &win32_window,
        InputState &input_state,
        VulkanInstance &instance);

    RenderContext(const RenderContext &) = delete;
    auto operator=(const RenderContext &) -> RenderContext & = delete;
    RenderContext(RenderContext &&) noexcept = delete;
    auto operator=(RenderContext &&) noexcept -> RenderContext & = delete;

    auto load_scene(std::string_view filename) -> Scene;

    auto update_and_render(Scene &scene) -> void; // or return a frame counter / diagnostic?

    auto shutdown() -> void;

    [[nodiscard]] auto renderer() const -> const VulkanRenderer &;
    [[nodiscard]] auto device() const -> const VulkanDevice &;

    auto init_debug_renderer(ImguiWrapper *debug_renderer) -> void;

  private:
    Win32Window &win32_window_;
    InputState &input_state_;

    std::chrono::steady_clock::time_point frame_begin_timestamp_;
    std::chrono::steady_clock::time_point last_window_recreate_time_;
    bool was_resize_pending_;

    VulkanSurface vulkan_surface_;
    VulkanDevice vulkan_device_;
    VulkanDescriptorPool vulkan_descriptor_pool_;
    ResourceManager resource_manager_;
    VulkanPipelineManager vulkan_pipeline_manager_;
    ResourceLoader resource_loader_;

    VulkanRenderer vulkan_renderer_;

    bool debug_enabled_ = false;
    ImguiWrapper *debug_renderer_ = nullptr;

  private:
    auto recreate_resources_() -> bool;
    auto init_() -> void;
}; // class RenderContext

} // namespace pong
