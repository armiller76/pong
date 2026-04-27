#include "engine/render_context.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <string>

#include "engine/engine_types.h"
#include "engine/resource_loader.h"
#include "engine/resource_manager.h"
#include "engine/vulkan/vulkan_descriptor_pool.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_instance.h"
#include "engine/vulkan/vulkan_pipeline_manager.h"
#include "engine/vulkan/vulkan_renderer.h"
#include "engine/vulkan/vulkan_surface.h"
#include "graphics/image.h"
#include "platform/win32_window.h"

namespace pong
{

using namespace std::literals;

class Camera;
class Scene;

RenderContext::RenderContext(const RenderContextInfo &render_context_info, Win32Window &win32_window)
    : app_name_{render_context_info.app_name}
    , engine_name_{render_context_info.engine_name}
    , version_{render_context_info.version}
    , win32_window_{win32_window}
    , vulkan_context_{}
    , vulkan_instance_{vulkan_context_, render_context_info}
    , vulkan_surface_{win32_window_.create_vulkan_surface(vulkan_instance_)}
    , vulkan_device_{vulkan_instance_, vulkan_surface_}
    , vulkan_descriptor_pool_{vulkan_device_, 2u} // TODO magic number (frames in flight)
    , resource_manager_{}
    , vulkan_pipeline_manager_{vulkan_device_, vulkan_descriptor_pool_, resource_manager_}
    , resource_loader_{vulkan_device_, resource_manager_, vulkan_pipeline_manager_, "c:/dev/pong/assets"sv} // TODO magic path (path to assets)
    , vulkan_renderer_{
          vulkan_device_,
          vulkan_surface_,
          resource_manager_,
          vulkan_pipeline_manager_,
          vulkan_descriptor_pool_,
          render_context_info.frames_in_flight,
          render_context_info.clear_color}
    , debug_renderer_{win32_window.win32_handles().window, vulkan_instance_, vulkan_device_, vulkan_renderer_,  render_context_info.project_root}
{
    init_();

    // book-keeping for dependencies that didn't exist when these objects were created
    vulkan_pipeline_manager_.set_color_attachment_format(vulkan_renderer_.swapchain_format());
    vulkan_pipeline_manager_.get_or_create_pipeline(vulkan_pipeline_manager_.get_default_pipeline_key());
}

auto RenderContext::load_scene(std::string_view filename) -> Scene
{
    return resource_loader_.loadgltf(filename);
}

auto RenderContext::update_and_render(Scene &scene, Camera &camera) -> void
{
    auto now = std::chrono::steady_clock::now();
    static auto last_window_recreate_time = now;

    static auto was_resize_pending = false;
    auto recreate = false;

    if (!win32_window_.is_minimized())
    {
        if (vulkan_renderer_.needs_recreate())
        {
            recreate = true;
        }
        else if (was_resize_pending && !win32_window_.resize_pending())
        {
            recreate = true;
        }
        else if (win32_window_.resize_pending() && now - last_window_recreate_time >= 50ms) // TODO Magic number
        {
            recreate = true;
        }
    }
    else
    {
        // TODO is this right?
        return;
    }

    was_resize_pending = win32_window_.resize_pending();

    if (recreate)
    {
        recreate_resources_();
        debug_renderer_.recreate();
        last_window_recreate_time = now;
    }

    debug_renderer_.begin_frame();
    debug_renderer_.render(); // calls ImGui::EndFrame() -- don't call manually

    scene.entities().at(scene.root_indices().at(0).value).rotate_by({0.0f, 0.0001f, 0.0f});

    vulkan_renderer_.render(scene, camera, debug_renderer_.get_draw_data());
}

auto RenderContext::shutdown() -> void
{
    vulkan_device_.native_handle().waitIdle();

    // TODO shutdown order?
    debug_renderer_resize_callback_ = nullptr;
    debug_renderer_.shutdown();
    vulkan_renderer_.shutdown();
    resource_manager_.shutdown();
}

// returns true if recreated, false if minimized
auto RenderContext::recreate_resources_() -> bool
{
    auto current_caps = vulkan_device_.physical_device().getSurfaceCapabilitiesKHR(vulkan_surface_.native_handle());
    if (current_caps.currentExtent.height == 0 || current_caps.currentExtent.width == 0)
    {
        return false;
    }

    // TODO should the context keep a list of resize callbacks?
    vulkan_device_.native_handle().waitIdle();

    vulkan_renderer_.recreate_resources();

    if (debug_renderer_resize_callback_)
    {
        debug_renderer_resize_callback_();
    }

    return true;
}

auto RenderContext::init_() -> void
{
    // create and upload fallback white texture
    constexpr auto white = std::array<std::uint8_t, 4>{255u, 255u, 255u, 255u};
    auto image = Image{"white_1x1", Extent2D{1u, 1u}, ImageFormat::RGBA8, white};
    resource_loader_.set_fallback_texture(resource_loader_.load("fallback", image));

    // load temporary default shaders
    resource_manager_.default_vertex_shader() =
        resource_loader_.load("simple.vert"sv, std::filesystem::path("c:/dev/Pong/assets/shaders/bin/simple_vert.spv"));
    resource_manager_.default_fragment_shader() =
        resource_loader_.load("simple.frag"sv, std::filesystem::path("c:/dev/Pong/assets/shaders/bin/simple_frag.spv"));
}

} // namespace pong
