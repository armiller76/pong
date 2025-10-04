#pragma once
#include <cstdint>
#include <string>
#include <string_view>

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
    auto application_name() const -> std::string_view;
    auto engine_name() const -> std::string_view;

  private:
    std::string application_name_;
    std::string engine_name_;
    ::vk::raii::Instance instance_;
};

}
