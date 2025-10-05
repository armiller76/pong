#include <cstdlib>
#include <print>
#include <string>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan_instance.h"
#include "engine/vulkan_swapchain.h"
#include "platform/win32_window.h"
#include "utils/exception.h"
#include "utils/log.h"

int main()
{
    std::string app_name{"Pong"};
    std::string engine_name{"NotAnEngine"};

    try
    {
        auto vk_context = ::vk::raii::Context();
        auto vk_instance = pong::VulkanInstance{
            vk_context,
            app_name,
            engine_name,
            static_cast<uint32_t>(APP_VERSION_MAJOR),
            static_cast<uint32_t>(APP_VERSION_MINOR),
            static_cast<uint32_t>(APP_VERSION_PATCH)};

        auto win32_window = pong::Win32Window(app_name, {100, 100, 800, 600});

        while (win32_window.running())
        {
            win32_window.process_events();
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
}
