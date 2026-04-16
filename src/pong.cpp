#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <print>
#include <string>
#include <string_view>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "core/entity.h"
#include "engine/resource_manager.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_instance.h"
#include "engine/vulkan/vulkan_renderer.h"
#include "engine/vulkan/vulkan_surface.h"
#include "graphics/camera.h"
#include "graphics/image.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "imgui/imgui_wrapper.h"
#include "math/rectangle.h"
#include "math/transform.h"
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

        auto resource_manager = pong::ResourceManager(vk_device);

        resource_manager.load(
            "simple.vert",
            std::filesystem::path(project_root + "/assets/shaders/bin/simple_vert.spv"),
            pong::ShaderStage::Vertex);
        resource_manager.load(
            "simple.frag",
            std::filesystem::path(project_root + "/assets/shaders/bin/simple_frag.spv"),
            pong::ShaderStage::Fragment);
        auto transform = pong::Transform{};

        auto test_triangle_mesh = resource_manager.load(std::move(pong::Mesh::create_test_triangle(vk_device)));
        transform.position = {0.5f, 0.0f, -5.0f};
        auto test_triangle = pong::Entity{"test_triangle", test_triangle_mesh, transform};

        auto test_rectangle_mesh = resource_manager.load(std::move(pong::Mesh::create_test_rectangle(vk_device)));
        transform.position = {-0.5f, 0.0f, -4.0f};
        auto test_rectangle = pong::Entity{"test_rectangle", test_rectangle_mesh, transform};

        auto test_gltf_mesh = resource_manager.load("gltf_mesh"sv, std::filesystem::path("assets/gltf/box/Box.glb"));
        transform.position = {0.0f, 0.5f, -3.0f};
        auto test_gltf_box = pong::Entity{"test_gltf_box", test_gltf_mesh, transform};

        auto entities = std::vector{
            //    test_triangle,
            //    test_rectangle,
            test_gltf_box,
        };

        //        auto image_1x1_white = pong::Image(
        //            "1x1_white"sv,
        //            pong::Extent2D{1u, 1u},
        //            pong::ImageFormat::RGBA8,
        //            std::vector<std::uint8_t>{255, 255, 255, 255});
        //        auto texture_1x1_white = resource_manager.load("texture_1x1_white", image_1x1_white);
        //        auto &the_actual_texture = resource_manager.get<pong::Texture2D>(texture_1x1_white);

        auto main_camera = pong::Camera();
        main_camera.set_position({5.0f, 0.0f, 5.0f});
        main_camera.set_view_target({0.0f, 0.0f, 0.0f});
        auto vk_renderer = pong::VulkanRenderer(vk_device, vk_surface, main_camera, resource_manager, 2u);
        auto imgui = pong::ImguiWrapper{window.win32_handles().window, vk_renderer, vk_instance, project_root};

        auto prev_time = std::chrono::high_resolution_clock::now();
        auto accum_time = 0.0f;
        while (!window.should_close())
        {
            auto start_time = std::chrono::high_resolution_clock::now();
            auto delta = std::chrono::duration<float>(start_time - prev_time);
            prev_time = start_time;

            window.process_events();

            imgui.begin_frame();
            imgui.render(); // calls ImGui::EndFrame()

            accum_time += delta.count();
            auto angle_x = accum_time * 0.25f;
            auto angle_y = accum_time * 0.33f;
            auto angle_z = accum_time * 0.5f;
            auto rotation = ::glm::quat(1, 0, 0, 0);
            rotation = ::glm::rotate(rotation, angle_x, {1, 0, 0});
            rotation = ::glm::rotate(rotation, angle_y, {0, 1, 0});
            rotation = ::glm::rotate(rotation, angle_z, {0, 0, 1});

            entities[0].set_rotation(rotation);

            vk_renderer.render(entities, imgui.get_draw_data());
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
