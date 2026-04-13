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

    auto command_buffer() -> ::vk::raii::CommandBuffer &;
    auto fence() const -> ::vk::Fence;
    auto wait_for_fence() const -> void;

  private:
    const VulkanDevice &device_;
    ::vk::raii::CommandPool pool_;
    ::vk::raii::CommandBuffer buffer_;
    ::vk::raii::Fence fence_;
};

}
