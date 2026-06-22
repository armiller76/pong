#include "core/application.h"

#include <memory>

#include "imgui/imgui_wrapper.h"

#include "core/scene.h"
#include "engine/input_state.h"
#include "engine/render_context.h"
#include "platform/win32_window.h"
#include "utils/log.h"

namespace pong
{

Application::Application(ApplicationInfo info)
    : app_info_{std::move(info)}
    , render_info_{RenderContextInfo{
          .frames_in_flight = 2u,
          .clear_color = {0.005f, 0.005f, 0.005f, 1.0f},
          .window_rect =
              {
                  .offset = {100u, 100u},
                  .extent = {800u, 600u},
              }}}
    , input_state_{}
    , win32_window_{app_info_.application_name, render_info_, input_state_}
    , vulkan_instance_{app_info_}
    , render_context_{render_info_, win32_window_, input_state_, vulkan_instance_}
    , debug_renderer_{nullptr}
{
    arm::log::debug("Application constructor");

    debug_renderer_ = std::make_unique<ImguiWrapper>(
        win32_window_.win32_handles().window,
        vulkan_instance_,
        render_context_,
        app_info_.application_root_dir.string());
    render_context_.init_debug_renderer(debug_renderer_.get());
}

Application::~Application() = default;

auto Application::run() -> void
{

    // auto scene = render_context.load_scene("assets/gltf/CesiumMilkTruck/CesiumMilkTruck.glb");
    // auto scene = render_context.load_scene("assets/gltf/BoomBox/BoomBox.glb");
    auto scene = render_context_.load_scene("assets/gltf/DamagedHelmet/DamagedHelmet.glb");
    scene.entities().at(scene.root_indices().at(0).value).scale_by({2.0f, 2.0f, 2.0f});
    scene.add_directional_light(
        {.direction_intensity = {-0.5f, -1.0f, -0.3f, 1.5f}, .color = {1.0f, 0.95f, 0.8f, 0.0f}});
    scene.add_directional_light({.direction_intensity = {0.6f, -0.5f, 0.4f, 0.4f}, .color = {0.6f, 0.7f, 1.0f, 0.0f}});
    scene.add_directional_light({.direction_intensity = {0.1f, -0.3f, 1.0f, 0.6f}, .color = {0.9f, 0.9f, 1.0f, 0.0f}});

    while (!win32_window_.should_close())
    {
        win32_window_.process_events();
        input_state_.process_events();
        render_context_.update_and_render(scene);
    }
    render_context_.shutdown();
}

} // namespace pong
