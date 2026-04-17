#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "utils/error.h"

namespace pong
{
class VulkanDevice;

class VulkanFrameCommandContext
{

  public:
    VulkanFrameCommandContext(const VulkanDevice &device, std::uint32_t frames_in_flight = 2u);
    ~VulkanFrameCommandContext() = default;

    VulkanFrameCommandContext(const VulkanFrameCommandContext &) = delete;
    VulkanFrameCommandContext &operator=(const VulkanFrameCommandContext &) = delete;
    VulkanFrameCommandContext(VulkanFrameCommandContext &&) = delete;
    VulkanFrameCommandContext &operator=(VulkanFrameCommandContext &&) = delete;

    auto wait_for_fence() -> void;
    auto advance_frame() -> void;

    auto current_command_buffer(this auto &&self) -> auto &&;
    auto current_fence(this auto &&self) -> auto &&;
    auto current_image_available_semaphore(this auto &&self) -> auto &&;

    auto frames_in_flight() const -> std::uint32_t;
    auto current_frame_index() const -> std::uint32_t;

  private:
    const VulkanDevice &device_;
    std::uint32_t frames_in_flight_;
    std::uint32_t current_frame_;

    ::vk::raii::CommandPool frame_command_pool_{nullptr};
    std::vector<::vk::raii::CommandBuffer> frame_command_buffers_;
    std::vector<::vk::raii::Fence> in_flight_fences_;
    std::vector<::vk::raii::Semaphore> image_available_semaphores_;
};

auto VulkanFrameCommandContext::current_command_buffer(this auto &&self) -> auto &&
{
    return self.frame_command_buffers_[self.current_frame_];
}

auto VulkanFrameCommandContext::current_fence(this auto &&self) -> auto &&
{
    return self.in_flight_fences_[self.current_frame_];
}

auto VulkanFrameCommandContext::current_image_available_semaphore(this auto &&self) -> auto &&
{
    return self.image_available_semaphores_[self.current_frame_];
}

}
