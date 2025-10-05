#include "vulkan_instance.h"

#include <array>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <vector>

#include "utils/error.h"

namespace pong
{

VulkanInstance::VulkanInstance(
    const ::vk::raii::Context &context,
    std::string_view application_name,
    std::string_view engine_name,
    uint32_t major_version,
    uint32_t minor_version,
    uint32_t patch_version)
    : instance_(
          context,
          [=]
          {
              auto required_extensions = std::vector{
                  VK_KHR_SURFACE_EXTENSION_NAME,
                  VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
                  VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
              auto available_extensions = ::vk::enumerateInstanceExtensionProperties();
              arm::ensure(
                  std::ranges::all_of(
                      required_extensions,
                      [&](const auto &required)
                      {
                          return std::ranges::find_if(
                                     available_extensions,
                                     [&](const auto &available) {
                                         return std::string_view(available.extensionName) == required;
                                     }) != available_extensions.end();
                      }),
                  "Required instance extension(s) not available");

              constexpr auto validation = std::array<const char *, 1>{"VK_LAYER_KHRONOS_validation"};
              auto available_layers = ::vk::enumerateInstanceLayerProperties();
              arm::ensure(
                  std::ranges::find_if(
                      available_layers,
                      [](const auto &available)
                      { return std::string_view(available.layerName) == validation[0]; }) != available_layers.end(),
                  "Validation layers not available");

              auto vk_application_info = VkApplicationInfo{};
              vk_application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
              vk_application_info.apiVersion = VK_API_VERSION_1_3;
              vk_application_info.applicationVersion = VK_MAKE_VERSION(major_version, minor_version, patch_version);
              vk_application_info.pApplicationName = application_name.data();
              vk_application_info.pEngineName = engine_name.data();

              auto vk_instance_create_info = VkInstanceCreateInfo{};
              vk_instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
              vk_instance_create_info.pApplicationInfo = &vk_application_info;
              vk_instance_create_info.enabledExtensionCount = static_cast<std::uint32_t>(required_extensions.size());
              vk_instance_create_info.ppEnabledExtensionNames = required_extensions.data();
              vk_instance_create_info.enabledLayerCount = 1;
              vk_instance_create_info.ppEnabledLayerNames = validation.data();

              return vk_instance_create_info;
          }())
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
