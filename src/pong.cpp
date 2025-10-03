#include <print>

#include <vulkan/vulkan.h>

int main()
{
    auto vk_application_info = VkApplicationInfo{};
    vk_application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vk_application_info.applicationVersion = VK_MAKE_VERSION(APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH);
    vk_application_info.pApplicationName = "Pong";
    vk_application_info.pEngineName = "NotAnEngine";

    auto vk_instance_create_info = VkInstanceCreateInfo{};
    vk_instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vk_instance_create_info.pApplicationInfo = &vk_application_info;

    auto vk_instance = VkInstance{};
    if (vkCreateInstance(&vk_instance_create_info, 0, &vk_instance) == VK_SUCCESS)
    {
        std::println("Successfully created Vulkan instance");
    }

    std::println("Hello Pong");
    return 0;
}
