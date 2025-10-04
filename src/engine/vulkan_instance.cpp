#include "vulkan_instance.h"

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
              auto vk_application_info = VkApplicationInfo{};
              vk_application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
              vk_application_info.apiVersion = VK_API_VERSION_1_3;
              vk_application_info.applicationVersion = VK_MAKE_VERSION(major_version, minor_version, patch_version);
              vk_application_info.pApplicationName = application_name.data();
              vk_application_info.pEngineName = engine_name.data();

              auto vk_instance_create_info = VkInstanceCreateInfo{};
              vk_instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
              vk_instance_create_info.pApplicationInfo = &vk_application_info;

              return vk_instance_create_info;
          }())
{
}

auto VulkanInstance::get() const -> const ::vk::raii::Instance &
{
    return instance_;
}

} // namespace pong
