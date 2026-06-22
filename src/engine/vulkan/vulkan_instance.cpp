#include "vulkan_instance.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "core/application.h"
#include "engine/vulkan/vulkan_utils.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

VulkanInstance::VulkanInstance(const ApplicationInfo &info)
    : application_name_{info.application_name}
    , engine_name_{info.engine_name}
    , context_{}
    , instance_{nullptr}
    , debug_messenger_{nullptr}
{
    arm::log::debug("VulkanInstance constructor");

    auto required_extensions = std::vector{
        ::vk::KHRSurfaceExtensionName,
        ::vk::KHRWin32SurfaceExtensionName,
        ::vk::EXTDebugUtilsExtensionName,
    };

    const auto [extension_property_result, available_extensions] = ::vk::enumerateInstanceExtensionProperties();
    arm::ensure(extension_property_result == ::vk::Result::eSuccess, "unable to get available instance extensions");

    arm::ensure(
        std::ranges::all_of(
            required_extensions,
            [&](const auto &required) -> auto
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
            [&validation](const auto &available) -> auto
            { return std::string_view(available.layerName) == validation[0]; })
            != available_layers.end(),
        "Validation layers not available");

    const auto app_engine_version = ::vk::makeVersion(info.version.major, info.version.minor, info.version.patch);
    const auto vk_application_info = ::vk::ApplicationInfo{
        .sType = ::vk::StructureType::eApplicationInfo,
        .pNext = nullptr,
        .pApplicationName = application_name_.c_str(),
        .applicationVersion = app_engine_version,
        .pEngineName = engine_name_.c_str(),
        .engineVersion = app_engine_version,
        .apiVersion = ::vk::ApiVersion13,
    };

    const auto vk_instance_create_info = ::vk::InstanceCreateInfo{
        .sType = ::vk::StructureType::eInstanceCreateInfo,
        .pNext = nullptr,
        .flags = {},
        .pApplicationInfo = &vk_application_info,
        .enabledLayerCount = 1,
        .ppEnabledLayerNames = validation.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(required_extensions.size()),
        .ppEnabledExtensionNames = required_extensions.data(),
    };

    auto create_instance_result = check_vk_expected(context_.createInstance(vk_instance_create_info));
    if (!create_instance_result.has_value())
    {
        throw arm::Exception("unable to create Vulkan instance");
    }
    instance_ = std::move(create_instance_result.value());

    const auto debug_messenger_create_info = ::vk::DebugUtilsMessengerCreateInfoEXT{
        .sType = ::vk::StructureType::eDebugUtilsMessengerCreateInfoEXT,
        .pNext = nullptr,
        .flags = {},
        .messageSeverity = ::vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
                           | ::vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                           | ::vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
        .messageType = ::vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                       | ::vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                       | ::vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
        .pfnUserCallback = &pong::VulkanInstance::vk_debug_callback,
        .pUserData = nullptr,
    };

    auto debug_messenger_result =
        check_vk_expected(instance_.createDebugUtilsMessengerEXT(debug_messenger_create_info, nullptr));
    if (!debug_messenger_result.has_value())
    {
        throw arm::Exception("unable to create Vulkan debug messenger");
    }
    debug_messenger_ = std::move(debug_messenger_result.value());
}

auto VulkanInstance::get() const -> const ::vk::raii::Instance &
{
    return instance_;
}

auto VulkanInstance::native_handle() const -> const ::vk::Instance
{
    return *instance_;
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
    return ::vk::False;
}

} // namespace pong
