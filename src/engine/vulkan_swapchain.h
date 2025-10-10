#pragma once

#include <vector>

#include <vulkan/vulkan_raii.hpp>

namespace pong
{
class VulkanDevice;
class VulkanSurface;

class VulkanSwapchain
{
  public:
    VulkanSwapchain(const VulkanSurface &surface, const VulkanDevice &device);

    auto recreate() -> void;

    auto get() const -> const ::vk::raii::SwapchainKHR &;

    auto format() const -> ::vk::Format;
    auto extent() const -> ::vk::Extent2D;
    auto images() const -> const std::vector<::vk::Image> &;
    auto image_views() const -> const std::vector<::vk::raii::ImageView> &;

  private:
    const VulkanDevice &device_;
    const VulkanSurface &surface_;

    ::vk::Format surface_format_{::vk::Format::eUndefined};
    ::vk::Extent2D extent_{};
    ::vk::ColorSpaceKHR color_space_{::vk::ColorSpaceKHR::eSrgbNonlinear};
    ::vk::PresentModeKHR present_mode_{::vk::PresentModeKHR::eFifo};

    ::vk::raii::SwapchainKHR swapchain_ = nullptr;
    std::vector<::vk::Image> images_;
    std::vector<::vk::raii::ImageView> image_views_;

  private:
    auto create_() -> void;
    auto destroy_views_() -> void;

    static auto choose_surface_format_(std::span<const ::vk::SurfaceFormatKHR> formats) -> ::vk::SurfaceFormatKHR;
    static auto choose_present_mode_(std::span<const ::vk::PresentModeKHR> modes) -> ::vk::PresentModeKHR;
    static auto choose_extent_(const ::vk::SurfaceCapabilitiesKHR &capabilities) -> ::vk::Extent2D;

}; // class VulkanSwapchain

} // namespace pong
