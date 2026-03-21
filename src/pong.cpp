#include <cstdlib>
#include <filesystem>
#include <print>
#include <string>

#include <vulkan/vulkan_raii.hpp>

#include "engine/resource_manager.h"
#include "engine/vulkan/vulkan_command_context.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_instance.h"
#include "engine/vulkan/vulkan_surface.h"
#include "engine/vulkan/vulkan_swapchain.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
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
        auto vk_context = ::vk::raii::Context();
        const auto vk_instance = pong::VulkanInstance{
            vk_context,
            app_name,
            engine_name,
            static_cast<uint32_t>(APP_VERSION_MAJOR),
            static_cast<uint32_t>(APP_VERSION_MINOR),
            static_cast<uint32_t>(APP_VERSION_PATCH)};

        // TODO: consider other platforms
        auto window = pong::Win32Window(app_name, window_offset, window_size);

        const auto vk_surface = window.create_vulkan_surface(vk_instance);
        const auto vk_device = pong::VulkanDevice(vk_instance, vk_surface);

        auto file = std::filesystem::path(project_root + "/assets/shaders/src/simple.vert");
        auto resources = pong::ResourceManager(vk_device);
        [[maybe_unused]] auto &simple_vertex_shader = resources.load("simple.vert", file, pong::ShaderStage::Vertex);
        [[maybe_unused]] auto &test_rectangle_mesh =
            resources.load(std::move(pong::Mesh::create_test_rectangle(vk_device)));

        while (!window.should_close())
        {
            window.process_events();
        }

        std::println("Hello Pong");
        return 0;
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
