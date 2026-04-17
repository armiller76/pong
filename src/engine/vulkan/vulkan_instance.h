#pragma once

#include <cstdint>
#include <string>

#include <vulkan/vulkan_raii.hpp>

namespace pong
{

class VulkanInstance
{
  public:
    VulkanInstance(
        const ::vk::raii::Context &context,
        std::string application_name,
        std::string engine_name,
        uint32_t major_version,
        uint32_t minor_version,
        uint32_t patch_version);

    auto get() const -> const ::vk::raii::Instance &;

  private:
    std::string application_name_;
    std::string engine_name_;
    ::vk::raii::Instance instance_;
    ::vk::raii::DebugUtilsMessengerEXT debug_messenger_;

    static VKAPI_ATTR auto VKAPI_CALL vk_debug_callback(
        ::vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
        ::vk::DebugUtilsMessageTypeFlagsEXT message_types,
        const ::vk::DebugUtilsMessengerCallbackDataEXT *callback_data,
        void *user_data) -> VkBool32;
};

}
