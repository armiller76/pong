#include "vulkan_instance.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "engine/engine_error.h"
#include "engine/engine_types.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

VulkanInstance::VulkanInstance(const ::vk::raii::Context &context, const RenderContextInfo &render_context_info)
    : application_name_{render_context_info.app_name}
    , engine_name_{render_context_info.engine_name}
    , instance_{nullptr}
    , debug_messenger_{nullptr}
{
    arm::log::debug("VulkanInstance constructor");

    auto required_extensions = std::vector{
        VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

    const auto [extension_property_result, available_extensions] = ::vk::enumerateInstanceExtensionProperties();
    arm::ensure(extension_property_result == ::vk::Result::eSuccess, "unable to get available instance extensions");

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
        "Required vulkan extension(s) not available");

    constexpr auto validation = std::array<const char *, 1>{"VK_LAYER_KHRONOS_validation"};
    const auto [layer_property_result, available_layers] = ::vk::enumerateInstanceLayerProperties();
    arm::ensure(layer_property_result == ::vk::Result::eSuccess, "unable to get available validation layers");

    arm::ensure(
        std::ranges::find_if(
            available_layers,
            [&validation](const auto &available) { return std::string_view(available.layerName) == validation[0]; })
            != available_layers.end(),
        "Validation layers not available");

    auto vk_application_info = ::vk::ApplicationInfo{};
    vk_application_info.sType = ::vk::StructureType::eApplicationInfo;
    vk_application_info.pNext = nullptr;
    vk_application_info.pApplicationName = application_name_.c_str();
    vk_application_info.applicationVersion = VK_MAKE_VERSION(
        render_context_info.version.major, render_context_info.version.minor, render_context_info.version.patch);
    vk_application_info.pEngineName = engine_name_.c_str();
    vk_application_info.apiVersion = VK_API_VERSION_1_3;

    auto vk_instance_create_info = ::vk::InstanceCreateInfo{};
    vk_instance_create_info.sType = ::vk::StructureType::eInstanceCreateInfo;
    vk_instance_create_info.pNext = nullptr;
    vk_instance_create_info.flags = {};
    vk_instance_create_info.pApplicationInfo = &vk_application_info;
    vk_instance_create_info.enabledLayerCount = 1;
    vk_instance_create_info.ppEnabledLayerNames = validation.data();
    vk_instance_create_info.enabledExtensionCount = static_cast<std::uint32_t>(required_extensions.size());
    vk_instance_create_info.ppEnabledExtensionNames = required_extensions.data();

    auto create_instance_result = check_vk_expected(context.createInstance(vk_instance_create_info));
    if (!create_instance_result)
    {
        throw arm::Exception("unable to create Vulkan instance");
    }
    instance_ = std::move(create_instance_result.value());

    auto debug_messenger_create_info = ::vk::DebugUtilsMessengerCreateInfoEXT{};
    debug_messenger_create_info.messageSeverity = ::vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
                                                  | ::vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                                                  | ::vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
    debug_messenger_create_info.messageType = ::vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                                              | ::vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                                              | ::vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    debug_messenger_create_info.pfnUserCallback = &pong::VulkanInstance::vk_debug_callback;
    auto debug_messenger_result =
        check_vk_expected(instance_.createDebugUtilsMessengerEXT(debug_messenger_create_info, nullptr));
    if (!debug_messenger_result)
    {
        throw arm::Exception("unable to create Vulkan debug messenger");
    }
    debug_messenger_ = std::move(debug_messenger_result.value());
}

auto VulkanInstance::native_handle() const -> const ::vk::raii::Instance &
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
