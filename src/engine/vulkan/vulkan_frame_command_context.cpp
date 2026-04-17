#include "vulkan_frame_command_context.h"

#include <cstdint>
#include <format>
#include <string>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "utils/error.h"
#include "vulkan_command_pool.h"
#include "vulkan_device.h"

namespace pong
{

VulkanFrameCommandContext::VulkanFrameCommandContext(const VulkanDevice &device, std::uint32_t frames_in_flight)
    : device_{device}
    , frames_in_flight_{frames_in_flight}
    , current_frame_{0}
{
    arm::log::debug("VulkanFrameCommandContext constructor");
#ifndef NDEBUG
    auto debug_name_info = ::vk::DebugUtilsObjectNameInfoEXT{};
    debug_name_info.sType = ::vk::StructureType::eDebugUtilsObjectNameInfoEXT;
    auto debug_name_str = std::string{};
#endif

    arm::ensure(frames_in_flight_ > 0, "frames_in_flight_ is <= 0");

    frame_command_pool_ = create_command_pool(device, command_context_type::Frame, "frame_command_pool");

    frame_command_buffers_.reserve(frames_in_flight_);
    auto buffers = create_command_buffers(device, "frame_command_buffer", frames_in_flight_, *frame_command_pool_);
    for (auto &buffer : buffers)
    {
        frame_command_buffers_.emplace_back(std::move(buffer));
    }

    in_flight_fences_.reserve(frames_in_flight_);
    image_available_semaphores_.reserve(frames_in_flight_);

    auto semaphore_info = ::vk::SemaphoreCreateInfo{};
    semaphore_info.sType = ::vk::StructureType::eSemaphoreCreateInfo;
    auto fence_info = ::vk::FenceCreateInfo{};
    fence_info.sType = ::vk::StructureType::eFenceCreateInfo;
    fence_info.flags = ::vk::FenceCreateFlagBits::eSignaled;
    for (std::uint32_t i = 0; i < frames_in_flight_; ++i)
    {
        in_flight_fences_.emplace_back(device_.native_handle(), fence_info);
        image_available_semaphores_.emplace_back(device_.native_handle(), semaphore_info);
#ifndef NDEBUG
        debug_name_str = std::format("Fence {}", i);
        debug_name_info.pObjectName = debug_name_str.c_str();
        debug_name_info.objectType = ::vk::ObjectType::eFence;
        debug_name_info.objectHandle = reinterpret_cast<std::uint64_t>(static_cast<::VkFence>(*in_flight_fences_[i]));
        device_.native_handle().setDebugUtilsObjectNameEXT(debug_name_info);

        debug_name_str = std::format("Image Available Semaphore {}", i);
        debug_name_info.pObjectName = debug_name_str.c_str();
        debug_name_info.objectType = ::vk::ObjectType::eSemaphore;
        debug_name_info.objectHandle =
            reinterpret_cast<std::uint64_t>(static_cast<::VkSemaphore>(*image_available_semaphores_[i]));
        device_.native_handle().setDebugUtilsObjectNameEXT(debug_name_info);
#endif
    }
}

auto VulkanFrameCommandContext::wait_for_fence() -> void
{
    auto result = device_.native_handle().waitForFences(*current_fence(), VK_TRUE, UINT64_MAX);
    arm::ensure(result == ::vk::Result::eSuccess, "Failed to wait for fence");

    device_.native_handle().resetFences(*current_fence());
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
