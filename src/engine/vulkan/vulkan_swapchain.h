#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

namespace pong
{
class VulkanDevice;
class VulkanSurface;

class VulkanSwapchain
{
  public:
    VulkanSwapchain(const VulkanDevice &device, const VulkanSurface &surface);

    auto recreate() -> void;

    auto native_handle() const -> const ::vk::raii::SwapchainKHR &;

    auto format(this auto &&self) -> auto &&;
    auto extent() const -> ::vk::Extent2D;

    auto images() const -> const std::vector<::vk::Image> &;
    auto image_views() const -> const std::vector<::vk::raii::ImageView> &;
    auto image_count() const -> std::uint32_t;
    auto semaphores() const -> const std::vector<::vk::raii::Semaphore> &;

  private:
    const VulkanDevice &device_;
    const VulkanSurface &surface_;
    ::vk::SurfaceCapabilitiesKHR capabilities_;
    std::vector<::vk::SurfaceFormatKHR> formats_;
    std::vector<::vk::PresentModeKHR> modes_;

    ::vk::Format surface_format_;
    ::vk::ColorSpaceKHR color_space_;
    ::vk::PresentModeKHR present_mode_;
    ::vk::Extent2D extent_;

    ::vk::raii::SwapchainKHR swapchain_;
    std::vector<::vk::Image> images_;
    std::vector<::vk::raii::ImageView> image_views_;
    std::vector<::vk::raii::Semaphore> render_finished_semaphores_;

  private:
    auto init_() -> void;
    auto create_() -> void;
    auto destroy_() -> void;

    static auto choose_surface_format_(std::span<const ::vk::SurfaceFormatKHR> formats) -> ::vk::SurfaceFormatKHR;
    static auto choose_present_mode_(std::span<const ::vk::PresentModeKHR> modes) -> ::vk::PresentModeKHR;
    static auto choose_extent_(const ::vk::SurfaceCapabilitiesKHR &capabilities) -> ::vk::Extent2D;

}; // class VulkanSwapchain

auto VulkanSwapchain::format(this auto &&self) -> auto &&
{
    return self.surface_format_;
}

} // namespace pong
