#include <chrono>
#include <cstdlib>
#include <string>
#include <string_view>

#include <vulkan/vulkan_raii.hpp>

#include "engine/resource_loader.h"
#include "engine/resource_manager.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_instance.h"
#include "engine/vulkan/vulkan_renderer.h"
#include "engine/vulkan/vulkan_surface.h"
#include "graphics/camera.h"
#include "graphics/color.h"
#include "graphics/mesh.h"
#include "imgui/imgui_wrapper.h"
#include "math/rectangle.h"
#include "platform/win32_window.h"
#include "utils/exception.h"
#include "utils/log.h"

int main()
{
    using namespace std::literals;

    auto project_root = std::string{"c:/dev/Pong"};
    auto app_name = std::string{"Pong"};
    auto engine_name = std::string{"NotAnEngine"};
    auto clear_color = pong::Color{0.42f, 0.42f, 0.42f, 1.04};

    auto window_rect = pong::Rectangle{.offset{100u, 100u}, .extent{800u, 600u}};

    try
    {
        arm::log::info("Hello Pong");

        auto vk_context = ::vk::raii::Context();
        const auto vk_instance = pong::VulkanInstance{
            vk_context,
            app_name,
            engine_name,
            static_cast<uint32_t>(APP_VERSION_MAJOR),
            static_cast<uint32_t>(APP_VERSION_MINOR),
            static_cast<uint32_t>(APP_VERSION_PATCH)};

        // TODO: other platforms
        auto window = pong::Win32Window(app_name, window_rect, clear_color);

        const auto vk_surface = window.create_vulkan_surface(vk_instance);
        const auto vk_device = pong::VulkanDevice(vk_instance, vk_surface);

        auto vk_renderer = pong::VulkanRenderer(vk_device, vk_surface, 2u, clear_color);
        auto scene = vk_renderer.load_scene("assets/gltf/CesiumMilkTruck/CesiumMilkTruck.glb");
        // auto scene = vk_renderer.load_scene("assets/gltf/BoomBox/BoomBox.glb");

        auto main_camera = pong::Camera();
        main_camera.set_position({0.0f, 2.0f, 10.0f});
        main_camera.set_view_target({0.0f, 0.0f, 0.0f});
        auto imgui = pong::ImguiWrapper{window.win32_handles().window, vk_renderer, vk_instance, project_root};

        auto last_window_recreate_time = std::chrono::high_resolution_clock::now();
        auto was_resize_pending = false;
        // auto accum_time = 0.0f;
        while (!window.should_close())
        {
            auto start_time = std::chrono::high_resolution_clock::now();
            auto renderer_needs_recreate = false;

            window.process_events();

            if (!window.is_minimized())
            {
                if (vk_renderer.needs_recreate())
                {
                    renderer_needs_recreate = true;
                }
                else if (was_resize_pending && !window.resize_pending())
                {
                    renderer_needs_recreate = true;
                }
                else if (window.resize_pending() && start_time - last_window_recreate_time >= 50ms) // TODO Magic number
                {
                    renderer_needs_recreate = true;
                }
            }
            else
            {
                continue;
            }

            was_resize_pending = window.resize_pending();

            if (renderer_needs_recreate)
            {
                vk_renderer.recreate_resources();
                last_window_recreate_time = start_time;
            }

            imgui.begin_frame();
            imgui.render(); // calls ImGui::EndFrame()

            // accum_time += delta.count();
            // auto angle_x = accum_time * 0.00025f;
            // auto angle_y = accum_time * 0.00033f;
            // auto angle_z = accum_time * 0.0005f;
            // auto rotation = ::glm::quat(1, 0, 0, 0);
            // rotation = ::glm::rotate(rotation, angle_x, {1, 0, 0});
            // rotation = ::glm::rotate(rotation, angle_y, {0, 1, 0});
            // rotation = ::glm::rotate(rotation, angle_z, {0, 0, 1});

            scene.entities().at(scene.root_indices().at(0).value).rotate_by({0.0f, 0.0001f, 0.0f});

            vk_renderer.render(scene, main_camera, imgui.get_draw_data());
        }
        vk_renderer.shutdown();
        return EXIT_SUCCESS;
    }
    catch (arm::Exception &e)
    {
        arm::log::error("Pong error: {}", e.to_string());
        return EXIT_FAILURE;
    }
    catch (...)
    {
        arm::log::error("Unknown error, exiting...");
        return EXIT_FAILURE;
    }
} // main()
