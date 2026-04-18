#include "vulkan_instance.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "utils/error.h"
#include "utils/log.h"

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
    arm::log::debug("VulkanInstance constructor");

    auto required_extensions = std::vector{
        VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    auto available_extensions = ::vk::enumerateInstanceExtensionProperties();
    arm::ensure(
        std::ranges::all_of(
            required_extensions,
            [&](const auto &required)
            {
                return std::ranges::find_if(
                           available_extensions,
                           [&](const auto &available) { return std::string_view(available.extensionName) == required; })
                       != available_extensions.end();
            }),
        "Required instance extension(s) not available");

    constexpr auto validation = std::array<const char *, 1>{"VK_LAYER_KHRONOS_validation"};
    auto available_layers = ::vk::enumerateInstanceLayerProperties();
    arm::ensure(
        std::ranges::find_if(
            available_layers,
            [&validation](const auto &available) { return std::string_view(available.layerName) == validation[0]; })
            != available_layers.end(),
        "Validation layers not available");

    auto vk_application_info = VkApplicationInfo{};
    vk_application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vk_application_info.apiVersion = VK_API_VERSION_1_3;
    vk_application_info.applicationVersion = VK_MAKE_VERSION(major_version, minor_version, patch_version);
    vk_application_info.pApplicationName = application_name_.c_str();
    vk_application_info.pEngineName = engine_name_.c_str();

    auto vk_instance_create_info = VkInstanceCreateInfo{};
    vk_instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vk_instance_create_info.pApplicationInfo = &vk_application_info;
    vk_instance_create_info.enabledExtensionCount = static_cast<std::uint32_t>(required_extensions.size());
    vk_instance_create_info.ppEnabledExtensionNames = required_extensions.data();
    vk_instance_create_info.enabledLayerCount = 1;
    vk_instance_create_info.ppEnabledLayerNames = validation.data();

    instance_ = ::vk::raii::Instance(context, vk_instance_create_info);

    auto debug_messenger_create_info = ::vk::DebugUtilsMessengerCreateInfoEXT{};
    debug_messenger_create_info.messageSeverity = ::vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
                                                  | ::vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                                                  | ::vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
    debug_messenger_create_info.messageType = ::vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                                              | ::vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                                              | ::vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    debug_messenger_create_info.pfnUserCallback = &pong::VulkanInstance::vk_debug_callback;
    debug_messenger_ = ::vk::raii::DebugUtilsMessengerEXT(instance_, debug_messenger_create_info, nullptr);
}

auto VulkanInstance::get() const -> const ::vk::raii::Instance &
{
    return instance_;
}

VKAPI_ATTR auto VKAPI_CALL VulkanInstance::vk_debug_callback(
    ::vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
    ::vk::DebugUtilsMessageTypeFlagsEXT message_types,
    const ::vk::DebugUtilsMessengerCallbackDataEXT *callback_data,
    [[maybe_unused]] void *user_data) -> VkBool32
{
    using Severity = ::vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using Type = ::vk::DebugUtilsMessageTypeFlagBitsEXT;

    auto severity = message_severity == Severity::eError     ? "ERROR"
                    : message_severity == Severity::eWarning ? "WARNING"
                                                             : "INFO";
    auto types = std::string{};
    if (message_types & Type::eGeneral)
    {
        types += "GENERAL|";
    }
    if (message_types & Type::eValidation)
    {
        types += "VALIDATION|";
    }
    if (message_types & Type::ePerformance)
    {
        types += "PERFORMANCE|";
    }
    if (!types.empty())
    {
        types.pop_back();
    }

    auto message = std::format(
        "Vulkan: [{}][{}] (ID: {}, Name: {}) {}",
        severity,
        types,
        callback_data->messageIdNumber,
        callback_data->pMessageIdName,
        callback_data->pMessage);

    switch (message_severity)
    {
        case Severity::eError:
        {
            arm::log::error("{}", message);
        }
        break;

        case Severity::eWarning:
        {
            arm::log::warn("{}", message);
        }
        break;
        case Severity::eInfo:
        {
            arm::log::info("{}", message);
        }
        break;
        default:
        {
            throw arm::Exception("Unknown Vk debug message severity");
        }
    }
    return VK_FALSE;
}

} // namespace pong
