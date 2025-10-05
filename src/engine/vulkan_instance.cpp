#include "vulkan_instance.h"

#include <array>
#include <cstdint>
#include <ranges>
#include <string>
#include <vector>

#include "utils/error.h"

namespace pong
{

VulkanInstance::VulkanInstance(
    const ::vk::raii::Context &context,
    std::string application_name,
    std::string engine_name,
    uint32_t major_version,
    uint32_t minor_version,
    uint32_t patch_version)
    : application_name_(application_name)
    , engine_name_(engine_name)
    , instance_({})
    , debug_messenger_({})
{
    auto debug_messenger_create_info = ::vk::DebugUtilsMessengerCreateInfoEXT{};
    debug_messenger_create_info.messageSeverity = ::vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                                                  ::vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                  ::vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
    debug_messenger_create_info.messageType = ::vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                              ::vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                              ::vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    debug_messenger_create_info.pfnUserCallback =
        reinterpret_cast<::vk::PFN_DebugUtilsMessengerCallbackEXT>(VulkanInstance::vk_debug_callback);
    debug_messenger_ = ::vk::raii::DebugUtilsMessengerEXT(instance_, debug_messenger_create_info, nullptr);
}

auto VulkanInstance::get() const -> const ::vk::raii::Instance &
{
    return instance_;
}

static auto VKAPI_PTR vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
    void *user_data) -> VkBool32
{
    // TODO: Implement this!
}

} // namespace pong
