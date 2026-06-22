#pragma once

#include <string>

#include <vulkan/vulkan_raii.hpp>

namespace pong
{

struct ApplicationInfo;

class VulkanInstance
{
  public:
    VulkanInstance(const ApplicationInfo &info);

    [[nodiscard]] auto get() const -> const ::vk::raii::Instance &;

    [[nodiscard]] auto native_handle() const -> const ::vk::Instance;

  private:
    const std::string application_name_;
    const std::string engine_name_;
    ::vk::raii::Context context_;
    ::vk::raii::Instance instance_;
    ::vk::raii::DebugUtilsMessengerEXT debug_messenger_;

    static VKAPI_ATTR auto VKAPI_CALL vk_debug_callback(
        ::vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
        ::vk::DebugUtilsMessageTypeFlagsEXT message_types,
        const ::vk::DebugUtilsMessengerCallbackDataEXT *callback_data,
        void *user_data) -> VkBool32;
};

}
