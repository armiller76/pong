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

    auto depth_image() const -> const ::vk::Image;
    auto depth_image_view() const -> const ::vk::ImageView;
    auto depth_format() const -> ::vk::Format;

  private:
    const VulkanDevice &device_;
    const VulkanSurface &surface_;

    ::vk::Format surface_format_{::vk::Format::eUndefined};
    ::vk::Extent2D extent_{};
    ::vk::ColorSpaceKHR color_space_{::vk::ColorSpaceKHR::eSrgbNonlinear};
    ::vk::PresentModeKHR present_mode_{::vk::PresentModeKHR::eFifo};

    ::vk::raii::SwapchainKHR swapchain_{nullptr};
    std::vector<::vk::Image> images_;
    std::vector<::vk::raii::ImageView> image_views_;

    ::vk::Format depth_format_{::vk::Format::eUndefined};
    ::vk::raii::Image depth_image_{nullptr};
    ::vk::raii::DeviceMemory depth_image_memory_{nullptr};
    ::vk::raii::ImageView depth_image_view_{nullptr};

  private:
    auto create_() -> void;
    auto destroy_() -> void;

    static auto choose_surface_format_(std::span<const ::vk::SurfaceFormatKHR> formats) -> ::vk::SurfaceFormatKHR;
    static auto choose_present_mode_(std::span<const ::vk::PresentModeKHR> modes) -> ::vk::PresentModeKHR;
    static auto choose_extent_(const ::vk::SurfaceCapabilitiesKHR &capabilities) -> ::vk::Extent2D;
    static auto choose_depth_format_(const ::vk::raii::PhysicalDevice &physical_device) -> ::vk::Format;

}; // class VulkanSwapchain

auto VulkanSwapchain::format(this auto &&self) -> auto &&
{
    return self.surface_format_;
}

} // namespace pong
