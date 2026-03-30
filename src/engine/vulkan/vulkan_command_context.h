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

    auto current_command_buffer() -> ::vk::raii::CommandBuffer &;
    auto current_command_buffer() const -> const ::vk::raii::CommandBuffer &;
    auto current_fence() -> ::vk::raii::Fence &;
    auto current_fence() const -> const ::vk::raii::Fence &;
    auto current_image_available_semaphore() -> ::vk::raii::Semaphore &;
    auto current_image_available_semaphore() const -> const ::vk::raii::Semaphore &;
    auto current_render_finished_semaphore() -> ::vk::raii::Semaphore &;
    auto current_render_finished_semaphore() const -> const ::vk::raii::Semaphore &;

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

}
