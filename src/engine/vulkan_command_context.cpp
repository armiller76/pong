#include "vulkan_command_context.h"

#include <cstddef>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "utils/error.h"
#include "vulkan_device.h"

namespace pong
{

VulkanCommandContext::VulkanCommandContext(const VulkanDevice &device, std::uint32_t frames_in_flight)
    : device_(device)
    , frames_in_flight_(frames_in_flight)
    , current_frame_(0)
{
    auto pool_create_info = ::vk::CommandPoolCreateInfo{};
    pool_create_info.flags = ::vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    pool_create_info.queueFamilyIndex = device_.graphics_queue_family_index();
    command_pool_ = ::vk::raii::CommandPool{device_.get(), pool_create_info};

    command_buffers_.reserve(frames_in_flight_);
    auto cb_allocate_info = ::vk::CommandBufferAllocateInfo{};
    cb_allocate_info.commandPool = *command_pool_;
    cb_allocate_info.level = ::vk::CommandBufferLevel::ePrimary;
    cb_allocate_info.commandBufferCount = frames_in_flight_;
    auto buffers = ::vk::raii::CommandBuffers{device_.get(), cb_allocate_info};
    for (auto &buffer : buffers)
    {
        command_buffers_.emplace_back(std::move(buffer));
    }

    image_available_semaphores_.reserve(frames_in_flight_);
    render_finished_semaphores_.reserve(frames_in_flight_);
    fences_.reserve(frames_in_flight_);

    auto semaphore_info = ::vk::SemaphoreCreateInfo{};
    auto fence_info = ::vk::FenceCreateInfo{};
    fence_info.flags = ::vk::FenceCreateFlagBits::eSignaled;
    for (std::size_t i = 0; i < frames_in_flight_; ++i)
    {
        image_available_semaphores_.emplace_back(device_.get(), semaphore_info);
        render_finished_semaphores_.emplace_back(device_.get(), semaphore_info);
        fences_.emplace_back(device_.get(), fence_info);
    }
}

auto VulkanCommandContext::wait_current_frame() -> void
{
    auto result = device_.get().waitForFences(*current_fence(), VK_TRUE, UINT64_MAX);
    arm::ensure(result == ::vk::Result::eSuccess, "Failed to wait for fence");

    device_.get().resetFences(*current_fence());
}

auto VulkanCommandContext::advance_frame() -> void
{
    current_frame_ = (current_frame_ + 1) % frames_in_flight_;
}

auto VulkanCommandContext::current_command_buffer() -> ::vk::raii::CommandBuffer &
{
    return command_buffers_[current_frame_];
}

auto VulkanCommandContext::current_command_buffer() const -> const ::vk::raii::CommandBuffer &
{
    return command_buffers_[current_frame_];
}

auto VulkanCommandContext::current_fence() -> ::vk::raii::Fence &
{
    return fences_[current_frame_];
}

auto VulkanCommandContext::current_fence() const -> const ::vk::raii::Fence &
{
    return fences_[current_frame_];
}

auto VulkanCommandContext::current_image_available_semaphore() -> ::vk::raii::Semaphore &
{
    return image_available_semaphores_[current_frame_];
}

auto VulkanCommandContext::current_image_available_semaphore() const -> const ::vk::raii::Semaphore &
{
    return image_available_semaphores_[current_frame_];
}

auto VulkanCommandContext::current_render_finished_semaphore() -> ::vk::raii::Semaphore &
{
    return render_finished_semaphores_[current_frame_];
}

auto VulkanCommandContext::current_render_finished_semaphore() const -> const ::vk::raii::Semaphore &
{
    return render_finished_semaphores_[current_frame_];
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
