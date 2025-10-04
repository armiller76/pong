#pragma once
#include <cstdint>
#include <string_view>

#include <vulkan/vulkan_raii.hpp>

namespace pong
{

class VulkanInstance
{
  public:
    VulkanInstance(
        const ::vk::raii::Context &context,
        std::string_view application_name,
        std::string_view engine_name,
        uint32_t major_version,
        uint32_t minor_version,
        uint32_t patch_version);

    auto get() const -> const ::vk::raii::Instance &;

  private:
    ::vk::raii::Instance instance_;
};

}
