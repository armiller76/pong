#include <chrono>
#include <cstdlib>
#include <filesystem>
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
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "imgui/imgui_wrapper.h"
#include "math/rectangle.h"
#include "platform/win32_window.h"
#include "utils/exception.h"
#include "utils/log.h"

int main()
{
    using namespace std::literals;

    std::string project_root{"c:/dev/Pong"};
    std::string app_name{"Pong"};
    std::string engine_name{"NotAnEngine"};

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
        auto window = pong::Win32Window(app_name, window_rect);

        const auto vk_surface = window.create_vulkan_surface(vk_instance);
        const auto vk_device = pong::VulkanDevice(vk_instance, vk_surface);

        auto resource_manager = pong::ResourceManager();
        auto resource_loader =
            pong::ResourceLoader(vk_device, resource_manager, "c:/dev/Pong/assets"sv); // TODO hardcoded path

        resource_loader.load(
            "simple.vert"sv,
            std::filesystem::path("c:/dev/Pong/assets/shaders/bin/simple_vert.spv"),
            pong::ShaderStage::Vertex);
        resource_loader.load(
            "simple.frag"sv,
            std::filesystem::path("c:/dev/Pong/assets/shaders/bin/simple_frag.spv"),
            pong::ShaderStage::Fragment);

        auto vk_renderer = pong::VulkanRenderer(vk_device, vk_surface, resource_manager, 2u);

        auto scene = resource_loader.loadgltf("assets/gltf/CesiumMilkTruck/CesiumMilkTruck.glb");
        // auto scene = resource_loader.loadgltf("assets/gltf/BoomBox/BoomBox.glb");

        auto main_camera = pong::Camera();
        main_camera.set_position({0.0f, 2.0f, 10.0f});
        main_camera.set_view_target({0.0f, 0.0f, 0.0f});
        auto imgui = pong::ImguiWrapper{window.win32_handles().window, vk_renderer, vk_instance, project_root};

        auto prev_time = std::chrono::high_resolution_clock::now();
        // auto accum_time = 0.0f;
        while (!window.should_close())
        {
            auto start_time = std::chrono::high_resolution_clock::now();
            // auto delta = std::chrono::duration<float>(start_time - prev_time);
            prev_time = start_time;

            window.process_events();

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

        return EXIT_SUCCESS;
    }
    catch (::vk::SystemError &e)
    {
        arm::log::error("Vulkan error: {}", e.what());
        return EXIT_FAILURE;
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
