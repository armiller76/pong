#include "shader.h"

#include <cstdint>
#include <ranges>
#include <span>
#include <string>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "utils/error.h"

namespace pong
{

Shader::Shader(std::string name, std::vector<std::uint32_t> spirv, ::vk::ShaderStageFlagBits stage)
    : name_(std::move(name))
    , spirv_(std::move(spirv))
    , stage_{stage}
{
    const auto valid_stages = std::vector{
        ::vk::ShaderStageFlagBits::eVertex, ::vk::ShaderStageFlagBits::eFragment, ::vk::ShaderStageFlagBits::eCompute};
    arm::ensure(std::ranges::contains(valid_stages, stage_), "{} is not a valid shader stage", ::vk::to_string(stage_));
    arm::ensure(!spirv_.empty(), "invalid shader (empty)");
    arm::ensure(!name_.empty(), "invalid shader name (empty)");
}

auto Shader::name() const -> const std::string &
{
    return name_;
}

auto Shader::spirv() const -> std::span<const std::uint32_t>
{
    return spirv_;
}

auto Shader::stage() const -> ::vk::ShaderStageFlagBits
{
    return stage_;
}

}
