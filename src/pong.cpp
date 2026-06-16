#include <string_view>

#include <vulkan/vulkan_raii.hpp>

#include "core/scene.h"
#include "engine/engine_utils.h"
#include "engine/input_state.h"
#include "engine/render_context.h"
#include "platform/win32_window.h"
#include "utils/exception.h"
#include "utils/log.h"


int main()
{
    using namespace std::literals;

    try
    {
        arm::log::info("Hello Pong");

        const auto render_context_info = pong::RenderContextInfo{
            .project_root = "c:/dev/Pong"sv,
            .app_name = "Pong"sv,
            .engine_name = "NotAnEngine"sv,
            .frames_in_flight = 2u,
            .clear_color = {0.005f, 0.005f, 0.005f, 1.0f},
            .window_rect = {.offset{100u, 100u}, .extent{800u, 600u}},
            .version = {.major = APP_VERSION_MAJOR, .minor = APP_VERSION_MINOR, .patch = APP_VERSION_PATCH}};

        auto input_state = pong::InputState();
        auto win32_window = pong::Win32Window(render_context_info, input_state);
        auto render_context = pong::RenderContext(render_context_info, win32_window, input_state);

        // auto scene = render_context.load_scene("assets/gltf/CesiumMilkTruck/CesiumMilkTruck.glb");
        // auto scene = render_context.load_scene("assets/gltf/BoomBox/BoomBox.glb");
        auto scene = render_context.load_scene("assets/gltf/DamagedHelmet/DamagedHelmet.glb");
        scene.entities().at(scene.root_indices().at(0).value).scale_by({2.0f, 2.0f, 2.0f});
        scene.add_directional_light({{-0.5f, -1.0f, -0.3f, 1.5f}, {1.0f, 0.95f, 0.8f, 0.0f}});
        scene.add_directional_light({{0.6f, -0.5f, 0.4f, 0.4f}, {0.6f, 0.7f, 1.0f, 0.0f}});
        scene.add_directional_light({{0.1f, -0.3f, 1.0f, 0.6f}, {0.9f, 0.9f, 1.0f, 0.0f}});

        while (!win32_window.should_close())
        {
            win32_window.process_events();
            input_state.process_events();
            render_context.update_and_render(scene);
        }
        render_context.shutdown();
        return 0;
    }
    catch (arm::Exception &e)
    {
        arm::log::error("Pong error: {}", e.to_string());
        return 1;
    }
    catch (...)
    {
        arm::log::error("Unknown error, exiting...");
        return 1;
    }
} // main()
