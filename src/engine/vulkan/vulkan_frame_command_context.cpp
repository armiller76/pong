#include "vulkan_frame_command_context.h"

#include <cstdint>
#include <format>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_command_pool.h"
#include "engine/vulkan/vulkan_device.h"
#include "utils/error.h"
#include "utils/exception.h"

namespace pong
{

VulkanFrameCommandContext::VulkanFrameCommandContext(const VulkanDevice &device, std::uint32_t frames_in_flight)
    : device_{device}
    , frames_in_flight_{frames_in_flight}
    , current_frame_{0}
{
    arm::log::debug("VulkanFrameCommandContext constructor");

    arm::ensure(frames_in_flight_ > 0, "frames_in_flight_ is <= 0");

    frame_command_pool_ = create_command_pool(device, CommandContextType::Frame, "frame_command_pool");

    frame_command_buffers_.reserve(frames_in_flight_);
    auto buffers = create_command_buffers(device, "frame_command_buffer", frames_in_flight_, *frame_command_pool_);
    for (auto &buffer : buffers)
    {
        frame_command_buffers_.emplace_back(std::move(buffer));
    }

    in_flight_fences_.reserve(frames_in_flight_);
    image_available_semaphores_.reserve(frames_in_flight_);

    const auto semaphore_info = ::vk::SemaphoreCreateInfo{
        .sType = ::vk::StructureType::eSemaphoreCreateInfo,
        .pNext = nullptr,
        .flags = {},
    };
    const auto fence_info = ::vk::FenceCreateInfo{
        .sType = ::vk::StructureType::eFenceCreateInfo,
        .pNext = nullptr,
        .flags = ::vk::FenceCreateFlagBits::eSignaled,
    };

    for (std::uint32_t i = 0; i < frames_in_flight_; ++i)
    {
        auto fence_result = check_vk_expected(device_.get().createFence(fence_info));
        if (!fence_result.has_value())
        {
            throw arm::Exception("unable to create fence");
        }

        auto semaphore_result = check_vk_expected(device_.get().createSemaphore(semaphore_info));
        if (!semaphore_result.has_value())
        {
            throw arm::Exception("unable to create semaphore");
        }

#ifndef NDEBUG
        auto debug_name_str = std::format("Fence {}", i);
        auto debug_name_info = ::vk::DebugUtilsObjectNameInfoEXT{
            .sType = ::vk::StructureType::eDebugUtilsObjectNameInfoEXT,
            .pNext = nullptr,
            .objectType = ::vk::ObjectType::eFence,
            .objectHandle = reinterpret_cast<std::uint64_t>(static_cast<::VkFence>(*fence_result.value())),
            .pObjectName = debug_name_str.c_str(),
        };
        device_.get().setDebugUtilsObjectNameEXT(debug_name_info);

        debug_name_str = std::format("Image Available Semaphore {}", i);
        debug_name_info.setPObjectName(debug_name_str.c_str());
        debug_name_info.setObjectType(::vk::ObjectType::eSemaphore);
        debug_name_info.setObjectHandle(
            reinterpret_cast<std::uint64_t>(static_cast<::VkSemaphore>(*semaphore_result.value())));
        device_.get().setDebugUtilsObjectNameEXT(debug_name_info);
#endif

        in_flight_fences_.push_back(std::move(fence_result.value()));
        image_available_semaphores_.push_back(std::move(semaphore_result.value()));
    }
}

auto VulkanFrameCommandContext::wait_for_fence() -> void
{
    auto result = device_.get().waitForFences(*current_fence(), ::vk::True, std::numeric_limits<std::uint64_t>::max());
    arm::ensure(result == ::vk::Result::eSuccess, "Failed to wait for fence");
}

auto VulkanFrameCommandContext::reset_fence() -> void
{
    device_.get().resetFences(*current_fence());
}
auto VulkanFrameCommandContext::advance_frame() -> void
{
    current_frame_ = (current_frame_ + 1) % frames_in_flight_;
}

auto VulkanFrameCommandContext::frames_in_flight() const -> std::uint32_t
{
    return frames_in_flight_;
}

auto VulkanFrameCommandContext::current_frame_index() const -> std::uint32_t
{
    return current_frame_;
}

}
