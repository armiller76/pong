#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <print>
#include <string>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "core/entity.h"
#include "engine/resource_manager.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_instance.h"
#include "engine/vulkan/vulkan_renderer.h"
#include "engine/vulkan/vulkan_surface.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "math/transform.h"
#include "platform/win32_window.h"
#include "utils/exception.h"
#include "utils/log.h"
#include "utils/util.h"

int main()
{
    std::string project_root{"c:/dev/Pong"};
    std::string app_name{"Pong"};
    std::string engine_name{"NotAnEngine"};

    auto window_offset = pong::Offset{100u, 100u};
    auto window_size = pong::Size{800u, 600u};

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
        auto window = pong::Win32Window(app_name, window_offset, window_size);

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
        auto test_triangle_mesh = resource_manager.load(std::move(pong::Mesh::create_test_triangle(vk_device)));
        auto test_rectangle_mesh = resource_manager.load(std::move(pong::Mesh::create_test_rectangle(vk_device)));
        auto default_transform = pong::Transform{};
        auto test_triangle = pong::Entity("test-triangle", test_triangle_mesh, default_transform);
        test_triangle.transform().position = {0.5f, 0.0f, 0.0f};
        auto test_rectangle = pong::Entity("test-rectangle", test_rectangle_mesh, default_transform);
        test_rectangle.transform().position = {-0.5f, 0.0f, 0.0f};
        auto entities = std::vector{test_triangle, test_rectangle};

        auto vk_renderer = pong::VulkanRenderer(vk_device, vk_surface, resource_manager, 2u);

        auto prev_time = std::chrono::high_resolution_clock::now();
        while (!window.should_close())
        {
            auto start_time = std::chrono::high_resolution_clock::now();
            auto delta = start_time - prev_time;
            prev_time = start_time;
            entities[0].rotate_by(
                ::glm::vec3(0, 0, ::glm::radians(std::chrono::duration<float>(delta).count() * 45.0f)));

            window.process_events();

            vk_renderer.render(entities);
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
