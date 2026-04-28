#include "vulkan_immediate_command_context.h"

#include <string_view>
#include <utility>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_command_pool.h"
#include "engine/vulkan/vulkan_device.h"
#include "utils/error.h"
#include "utils/log.h"


namespace pong

{

VulkanImmediateCommandContext::VulkanImmediateCommandContext(const VulkanDevice &device, std::string_view name)
    : device_{device}
    , pool_{create_command_pool(device_, command_context_type::Immediate, name)}
    , buffer_{std::move(create_command_buffers(device_, name, 1u, *pool_).at(0))}
    , fence_{[&]()
             {
                 auto fence_result = check_vk_expected(device_.native_handle().createFence(::vk::FenceCreateInfo{}));
                 if (!fence_result)
                 {
                     throw arm::Exception("unable to create fence");
                 }
                 return std::move(fence_result.value());
             }()}

{
    arm::log::debug("VulkanImmediateCommandContext constructor");
}

auto VulkanImmediateCommandContext::command_buffer() -> ::vk::raii::CommandBuffer &
{
    return buffer_;
}

auto VulkanImmediateCommandContext::fence() const -> ::vk::Fence
{
    return *fence_;
}

auto VulkanImmediateCommandContext::wait_for_fence() const -> void
{
    auto result = device_.native_handle().waitForFences(*fence_, VK_TRUE, UINT64_MAX);
    arm::ensure(result == ::vk::Result::eSuccess, "Failed to wait for fence");

    device_.native_handle().resetFences(*fence_);
}

} // namespace pong
