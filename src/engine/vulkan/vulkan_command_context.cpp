#include "vulkan_command_context.h"

#include <cstdint>
#include <format>
#include <string>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "utils/error.h"
#include "vulkan_device.h"

namespace pong
{

VulkanCommandContext::VulkanCommandContext(
    const VulkanDevice &device,
    std::uint32_t swap_chain_image_count,
    std::uint32_t frames_in_flight)
    : device_{device}
    , frames_in_flight_{frames_in_flight}
    , swap_chain_image_count_{swap_chain_image_count}
    , current_frame_{0}
{
    arm::log::debug("VulkanCommandContext constructor");
#ifndef NDEBUG
    auto debug_name_info = ::vk::DebugUtilsObjectNameInfoEXT{};
    auto debug_name_str = std::string{};
#endif

    arm::ensure(frames_in_flight_ > 0, "frames_in_flight_ is <= 0");

    auto pool_create_info = ::vk::CommandPoolCreateInfo{};
    pool_create_info.flags = ::vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    pool_create_info.queueFamilyIndex = device_.graphics_queue_family_index();
    command_pool_ = ::vk::raii::CommandPool{device_.native_handle(), pool_create_info};
#ifndef NDEBUG
    debug_name_str = "Graphics Command Pool";
    debug_name_info.pObjectName = debug_name_str.c_str();
    debug_name_info.objectType = ::vk::ObjectType::eCommandPool;
    debug_name_info.objectHandle = reinterpret_cast<std::uint64_t>(static_cast<::VkCommandPool>(*command_pool_));
    device_.native_handle().setDebugUtilsObjectNameEXT(debug_name_info);
#endif

    command_buffers_.reserve(frames_in_flight_);
    auto cb_allocate_info = ::vk::CommandBufferAllocateInfo{};
    cb_allocate_info.commandPool = *command_pool_;
    cb_allocate_info.level = ::vk::CommandBufferLevel::ePrimary;
    cb_allocate_info.commandBufferCount = frames_in_flight_;
    auto buffers = ::vk::raii::CommandBuffers{device_.native_handle(), cb_allocate_info};
    for (auto &buffer : buffers)
    {
        command_buffers_.emplace_back(std::move(buffer));
    }
#ifndef NDEBUG
    for (std::uint32_t i = 0; i < frames_in_flight_; ++i)
    {
        debug_name_str = std::format("Graphics Command Buffer {}", i);
        debug_name_info.pObjectName = debug_name_str.c_str();
        debug_name_info.objectType = ::vk::ObjectType::eCommandBuffer;
        debug_name_info.objectHandle =
            reinterpret_cast<std::uint64_t>(static_cast<::VkCommandBuffer>(*command_buffers_[i]));
        device_.native_handle().setDebugUtilsObjectNameEXT(debug_name_info);
    }
#endif

    fences_.reserve(frames_in_flight_);
    image_available_semaphores_.reserve(frames_in_flight_);
    render_finished_semaphores_.reserve(swap_chain_image_count_);

    auto semaphore_info = ::vk::SemaphoreCreateInfo{};
    auto fence_info = ::vk::FenceCreateInfo{};
    fence_info.flags = ::vk::FenceCreateFlagBits::eSignaled;
    for (std::uint32_t i = 0; i < frames_in_flight_; ++i)
    {
        fences_.emplace_back(device_.native_handle(), fence_info);
        image_available_semaphores_.emplace_back(device_.native_handle(), semaphore_info);
#ifndef NDEBUG
        debug_name_str = std::format("Fence {}", i);
        debug_name_info.pObjectName = debug_name_str.c_str();
        debug_name_info.objectType = ::vk::ObjectType::eFence;
        debug_name_info.objectHandle = reinterpret_cast<std::uint64_t>(static_cast<::VkFence>(*fences_[i]));
        device_.native_handle().setDebugUtilsObjectNameEXT(debug_name_info);

        debug_name_str = std::format("Image Available Semaphore {}", i);
        debug_name_info.pObjectName = debug_name_str.c_str();
        debug_name_info.objectType = ::vk::ObjectType::eSemaphore;
        debug_name_info.objectHandle =
            reinterpret_cast<std::uint64_t>(static_cast<::VkSemaphore>(*image_available_semaphores_[i]));
        device_.native_handle().setDebugUtilsObjectNameEXT(debug_name_info);
#endif
    }

    for (std::uint32_t i = 0; i < swap_chain_image_count_; ++i)
    {
        render_finished_semaphores_.emplace_back(device_.native_handle(), semaphore_info);
#ifndef NDEBUG
        debug_name_str = std::format("Render Finished Semaphore {}", i);
        debug_name_info.pObjectName = debug_name_str.c_str();
        debug_name_info.objectType = ::vk::ObjectType::eSemaphore;
        debug_name_info.objectHandle =
            reinterpret_cast<std::uint64_t>(static_cast<::VkSemaphore>(*render_finished_semaphores_[i]));
        device_.native_handle().setDebugUtilsObjectNameEXT(debug_name_info);
#endif
    }
}

auto VulkanCommandContext::wait_current_frame() -> void
{
    auto result = device_.native_handle().waitForFences(*current_fence(), VK_TRUE, UINT64_MAX);
    arm::ensure(result == ::vk::Result::eSuccess, "Failed to wait for fence");

    device_.native_handle().resetFences(*current_fence());
}

auto VulkanCommandContext::advance_frame() -> void
{
    current_frame_ = (current_frame_ + 1) % frames_in_flight_;
}

auto VulkanCommandContext::frames_in_flight() const -> std::uint32_t
{
    return frames_in_flight_;
}

auto VulkanCommandContext::current_frame_index() const -> std::uint32_t
{
    return current_frame_;
}

}
