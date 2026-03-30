#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

namespace pong
{
class VulkanDevice;

class VulkanCommandContext
{

  public:
    VulkanCommandContext(const VulkanDevice &device, std::uint32_t frames_in_flight = 2u);
    ~VulkanCommandContext() = default;

    VulkanCommandContext(const VulkanCommandContext &) = delete;
    VulkanCommandContext &operator=(VulkanCommandContext &&) = delete;
    VulkanCommandContext(VulkanCommandContext &&) = delete;
    VulkanCommandContext &operator=(const VulkanCommandContext &&) = delete;

    auto wait_current_frame() -> void;
    auto advance_frame() -> void;

    auto current_command_buffer(this auto &&self) -> auto &&;
    auto current_fence(this auto &&self) -> auto &&;
    auto current_image_available_semaphore(this auto &&self) -> auto &&;
    auto current_render_finished_semaphore(this auto &&self) -> auto &&;

    auto frames_in_flight() const -> std::uint32_t;
    auto current_frame_index() const -> std::uint32_t;

  private:
    const VulkanDevice &device_;
    std::uint32_t frames_in_flight_{};
    std::uint32_t current_frame_{};

    ::vk::raii::CommandPool command_pool_{nullptr};
    std::vector<::vk::raii::CommandBuffer> command_buffers_;
    std::vector<::vk::raii::Fence> fences_;
    std::vector<::vk::raii::Semaphore> image_available_semaphores_;
    std::vector<::vk::raii::Semaphore> render_finished_semaphores_;
};

auto VulkanCommandContext::current_command_buffer(this auto &&self) -> auto &&
{
    return self.command_buffers_[self.current_frame_];
}

auto VulkanCommandContext::current_fence(this auto &&self) -> auto &&
{
    return self.fences_[self.current_frame_];
}

auto VulkanCommandContext::current_image_available_semaphore(this auto &&self) -> auto &&
{
    return self.image_available_semaphores_[self.current_frame_];
}

auto VulkanCommandContext::current_render_finished_semaphore(this auto &&self) -> auto &&
{
    return self.render_finished_semaphores_[self.current_frame_];
}

}
