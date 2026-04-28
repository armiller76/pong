#pragma once

#include "engine/engine_types.h"
#include <string>

#include <vulkan/vulkan_raii.hpp>

namespace pong
{

class VulkanInstance
{
  public:
    VulkanInstance(const ::vk::raii::Context &context, const RenderContextInfo &render_context_info);

    auto native_handle() const -> const ::vk::raii::Instance &;

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
