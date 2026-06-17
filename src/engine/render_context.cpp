#include "engine/render_context.h"

#include <chrono>
#include <string>

#include "core/scene.h"
#include "engine/engine_utils.h"
#include "engine/input_state.h"
#include "engine/resource_loader.h"
#include "engine/resource_manager.h"
#include "engine/vulkan/vulkan_descriptor_pool.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_instance.h"
#include "engine/vulkan/vulkan_pipeline_manager.h"
#include "engine/vulkan/vulkan_renderer.h"
#include "engine/vulkan/vulkan_surface.h"
#include "graphics/free_look_camera.h"
#include "graphics/image.h"
#include "platform/win32_window.h"

namespace pong
{

using namespace std::literals;

class Scene;

RenderContext::RenderContext(const RenderContextInfo &render_context_info, Win32Window &win32_window, InputState &input_state)
    : app_name_{render_context_info.app_name}
    , engine_name_{render_context_info.engine_name}
    , version_{render_context_info.version}
    , win32_window_{win32_window}
    , input_state_{input_state}
    , last_window_recreate_time_{std::chrono::steady_clock::now()}
    , was_resize_pending_{false}
    , frame_begin_timestamp_{std::chrono::steady_clock::now()}
    , vulkan_context_{}
    , vulkan_instance_{vulkan_context_, render_context_info}
    , vulkan_surface_{vulkan_instance_, win32_window_.win32_handles()}
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
}

auto RenderContext::load_scene(std::string_view filename) -> Scene
{
    auto entity_info = resource_loader_.loadgltf(filename);
    return {
        std::move(entity_info.first),
        std::move(entity_info.second),
        vulkan_renderer_.swapchain_extent().width,
        vulkan_renderer_.swapchain_extent().height,
    };
}

auto RenderContext::update_and_render(Scene &scene) -> void
{
    auto last_frame_timestamp = frame_begin_timestamp_;
    frame_begin_timestamp_ = std::chrono::steady_clock::now();
    auto frame_delta_seconds = std::chrono::duration<float>(frame_begin_timestamp_ - last_frame_timestamp);

    // TODO if resize logic goes haywire, then i guess the next line was needed, but why?!
    // was_resize_pending_ = false;
    auto should_recreate = false;

    if (!win32_window_.is_minimized())
    {
        if ((vulkan_renderer_.needs_recreate())                         //
            || (was_resize_pending_ && !win32_window_.resize_pending()) //
            || (win32_window_.resize_pending() && (frame_begin_timestamp_ - last_window_recreate_time_ >= 50ms)))
        {
            should_recreate = true;
        }
    }
    else
    {
        return;
    }

    was_resize_pending_ = win32_window_.resize_pending();

    if (should_recreate)
    {
        recreate_resources_();
        debug_renderer_.recreate();
        last_window_recreate_time_ = frame_begin_timestamp_;

        const auto new_extent = vulkan_renderer_.swapchain_extent();
        scene.frame_camera().resize(new_extent.width, new_extent.height);
    }

    const auto translate_speed = 10.0f;
    const auto mouse_sensitivity = 0.0005f;

    // scene.entities().at(scene.root_indices().at(0).value).rotate_by({0.0f, 0.01f, 0.0f});
    scene.frame_camera().translate_by(
        input_state_.move_direction(scene.frame_camera()) * (frame_delta_seconds.count() * translate_speed));
    scene.frame_camera().adjust_pitch(-input_state_.mouse_state().frame_delta_y * mouse_sensitivity);
    scene.frame_camera().adjust_yaw(input_state_.mouse_state().frame_delta_x * mouse_sensitivity);

    debug_renderer_.render(); // calls ImGui::BeginFrame() and ImGui::EndFrame() -- don't call manually

    vulkan_renderer_.render(scene, debug_renderer_.get_draw_data());

    input_state_.advance_frame();
}

auto RenderContext::shutdown() -> void
{
    vulkan_device_.get().waitIdle();

    // TODO shutdown order?
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

    vulkan_device_.get().waitIdle();

    vulkan_renderer_.recreate_resources();
    debug_renderer_.recreate();

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

    // book-keeping for pipeline manager dependencies
    vulkan_pipeline_manager_.set_color_attachment_format(vulkan_renderer_.swapchain_format());
    vulkan_pipeline_manager_.get_or_create_pipeline(vulkan_pipeline_manager_.get_default_pipeline_key());
}

} // namespace pong
