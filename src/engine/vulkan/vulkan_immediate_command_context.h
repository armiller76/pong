#pragma once

#include <string_view>

#include <vulkan/vulkan_raii.hpp>

namespace pong
{
class VulkanDevice;

class VulkanImmediateCommandContext
{
  public:
    VulkanImmediateCommandContext(const VulkanDevice &device, std::string_view name);
    ~VulkanImmediateCommandContext() = default;

    VulkanImmediateCommandContext(const VulkanImmediateCommandContext &) = delete;
    auto operator=(const VulkanImmediateCommandContext &) -> VulkanImmediateCommandContext = delete;
    VulkanImmediateCommandContext(VulkanImmediateCommandContext &&) noexcept = default;
    auto operator=(VulkanImmediateCommandContext &&) noexcept -> VulkanImmediateCommandContext & = delete;

    auto command_buffer() const -> ::vk::CommandBuffer;
    auto fence() const -> ::vk::Fence;

  private:
    const VulkanDevice &device_;
    ::vk::raii::CommandPool pool_;
    ::vk::raii::CommandBuffer buffer_;
    ::vk::raii::Fence fence_;
};

}
