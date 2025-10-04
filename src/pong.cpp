#include <cstdlib>
#include <print>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan_instance.h"
#include "utils/exception.h"
#include "utils/log.h"

int main()
{
    try
    {
        auto vk_context = ::vk::raii::Context();
        auto vk_instance = pong::VulkanInstance{
            vk_context,
            "Pong",
            "NotAnEngine",
            static_cast<uint32_t>(APP_VERSION_MAJOR),
            static_cast<uint32_t>(APP_VERSION_MINOR),
            static_cast<uint32_t>(APP_VERSION_PATCH)};
        std::println("Hello Pong");
        return 0;
    }
    catch (::vk::SystemError &e)
    {
        pong::log::error("Vulkan error: {}", e.what());
    }
    catch (pong::Exception &e)
    {
        pong::log::error("Pong error: {}", e.to_string());
    }
    catch (...)
    {
        pong::log::error("Unknown error, exiting...");
        return EXIT_FAILURE;
    }
}
