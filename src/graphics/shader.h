#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "engine/engine_error.h"
#include "engine/vulkan/vulkan_device.h"
#include "utils/exception.h"

namespace pong
{

enum class ShaderStage
{
    Vertex,
    Fragment,
    Compute
};

class Shader
{
  public:
    Shader(
        const VulkanDevice &device,
        std::string_view name,
        std::vector<std::uint32_t> spirv,
        std::string_view entry_point = "main")
        : name_{name}
        , spirv_{std::move(spirv)}
        , module_{[&]
                  {
                      auto create_info = ::vk::ShaderModuleCreateInfo{};
                      create_info.sType = ::vk::StructureType::eShaderModuleCreateInfo;
                      create_info.codeSize = spirv_view().size_bytes();
                      create_info.pCode = spirv_view().data();
                      auto result = check_vk_expected(device.native_handle().createShaderModule(create_info));
                      if (!result)
                      {
                          // TODO avoid exception once error handling is available
                          throw arm::Exception(
                              "error: {} (\"{}\")", to_string(result.error().code), result.error().message);
                      }
                      return std::move(result.value());
                  }()}
        , entry_point_{entry_point}
    {
    }

    auto spirv_view() const -> std::span<const std::uint32_t>
    {
        return {spirv_.data(), spirv_.data() + spirv_.size()};
    }

    auto name() const -> std::string_view
    {
        return name_;
    }

    auto module_handle() const -> ::vk::ShaderModule
    {
        return *module_;
    }

    auto entry_point_c_str() const -> const char *
    {
        return entry_point_.c_str();
    }

  private:
    std::string name_;
    std::vector<std::uint32_t> spirv_;
    ::vk::raii::ShaderModule module_;
    std::string entry_point_;

}; // class Shader

} // namespace pong
