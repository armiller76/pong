#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace vk
{
enum class ShaderStageFlagBits;
}

namespace pong
{

class Shader
{
  public:
    explicit Shader(std::string name, std::vector<std::uint32_t> spirv, ::vk::ShaderStageFlagBits stage);

    auto name() const -> const std::string &;
    auto spirv() const -> std::span<const std::uint32_t>;
    auto stage() const -> ::vk::ShaderStageFlagBits;

  private:
    std::string name_;
    std::vector<std::uint32_t> spirv_;
    ::vk::ShaderStageFlagBits stage_;
};

}
