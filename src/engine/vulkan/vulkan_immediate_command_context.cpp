#include "vulkan_immediate_command_context.h"

#include <string_view>

#include <vulkan/vulkan_raii.hpp>

#include "utils/log.h"
#include "vulkan_command_pool.h"
#include "vulkan_device.h"

namespace pong

{

VulkanImmediateCommandContext::VulkanImmediateCommandContext(const VulkanDevice &device, std::string_view name)
    : device_{device}
    , pool_{create_command_pool(device_, command_context_type::Immediate, name)}
    , buffer_{std::move(create_command_buffers(device_, name, 1u, *pool_).at(0))}
    , fence_{[&]()
             {
                 auto fence_info = ::vk::FenceCreateInfo{};
                 fence_info.sType = ::vk::StructureType::eFenceCreateInfo;
                 fence_info.flags = ::vk::FenceCreateFlagBits::eSignaled;
                 return ::vk::raii::Fence(device_.native_handle(), fence_info);
             }()}

{
    arm::log::debug("VulkanImmediateCommandContext constructor");
}

auto VulkanImmediateCommandContext::command_buffer() const -> ::vk::CommandBuffer
{
    return *buffer_;
}

auto VulkanImmediateCommandContext::fence() const -> ::vk::Fence
{
    return *fence_;
}

} // namespace pong
