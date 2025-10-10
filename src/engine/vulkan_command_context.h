#pragma once

#include <cstddef>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

namespace pong
{
class VulkanDevice;

class VulkanCommandContext
{

  public:
    VulkanCommandContext(const VulkanDevice &device, std::uint32_t frames_in_flight = 2u);

    auto wait_current_frame() -> void;
    auto advance_frame() -> void;

    auto current_command_buffer() -> ::vk::raii::CommandBuffer &;
    auto current_fence() -> ::vk::raii::Fence &;
    auto current_image_available_semaphore() -> ::vk::raii::Semaphore &;
    auto current_render_finished_semaphore() -> ::vk::raii::Semaphore &;

    auto frames_in_flight() -> std::uint32_t;
    auto current_frame_index() -> std::uint32_t;

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

}
