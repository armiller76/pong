#include "vulkan_immediate_command_context.h"

#include <limits>
#include <string_view>
#include <utility>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_command_pool.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_utils.h"
#include "utils/error.h"
#include "utils/log.h"

namespace pong

{

VulkanImmediateCommandContext::VulkanImmediateCommandContext(const VulkanDevice &device, std::string_view name)
    : device_{device}
    , pool_{create_command_pool(device_, CommandContextType::Immediate, name)}
    , buffer_{std::move(create_command_buffers(device_, name, 1u, *pool_)[0])}
    , fence_{
          [&]() -> ::vk::raii::Fence
          {
              auto fence_result = check_vk_expected(device_.get().createFence(::vk::FenceCreateInfo{}));
              if (!fence_result.has_value())
              {
                  throw arm::Exception("unable to create fence");
              }
              return std::move(fence_result.value());
          }()}

{
    arm::log::debug("VulkanImmediateCommandContext constructor '{}'", name);
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
    auto result = device_.get().waitForFences(*fence_, ::vk::True, std::numeric_limits<std::uint64_t>::max());
    arm::ensure(result == ::vk::Result::eSuccess, "Failed to wait for fence");

    device_.get().resetFences(fence());
}

} // namespace pong
