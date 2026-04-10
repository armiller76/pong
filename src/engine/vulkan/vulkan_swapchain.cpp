#include "vulkan_swapchain.h"

#include <cstdint>
#include <ranges>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "utils/exception.h"
#include "utils/log.h"
#include "vulkan_device.h"
#include "vulkan_surface.h"

namespace pong
{

VulkanSwapchain::VulkanSwapchain(const VulkanDevice &device, const VulkanSurface &surface)
    : device_{device}
    , surface_{surface}
{
    arm::log::debug("VulkanSwapchain constructor");
    create_();
}

auto VulkanSwapchain::recreate() -> void
{
    destroy_();
    create_();
}

auto VulkanSwapchain::native_handle() const -> const ::vk::raii::SwapchainKHR &
{
    return swapchain_;
}

auto VulkanSwapchain::extent() const -> ::vk::Extent2D
{
    return extent_;
}

auto VulkanSwapchain::images() const -> const std::vector<::vk::Image> &
{
    return images_;
}

auto VulkanSwapchain::image_views() const -> const std::vector<::vk::raii::ImageView> &
{
    return image_views_;
}

auto VulkanSwapchain::image_count() const -> std::uint32_t
{
    return static_cast<std::uint32_t>(images_.size());
}

auto VulkanSwapchain::depth_image() const -> const ::vk::Image
{
    return *depth_image_;
}

auto VulkanSwapchain::depth_image_view() const -> const ::vk::ImageView
{
    return *depth_image_view_;
}

auto VulkanSwapchain::depth_format() const -> ::vk::Format
{
    return depth_format_;
}

auto VulkanSwapchain::create_() -> void
{
    // TODO Bloated function

    // grab all the data we need from the device up front
    const auto capabilities = device_.physical_device().getSurfaceCapabilitiesKHR(surface_.get());
    const auto surface_formats = device_.physical_device().getSurfaceFormatsKHR(surface_.get());
    const auto modes = device_.physical_device().getSurfacePresentModesKHR(surface_.get());

    // if the device doesn't give us any surface formats or present modes, crash
    if (surface_formats.empty() || modes.empty())
    {
        throw arm::Exception("No surface formats or present modes or both");
    }

    // TODO consider parameterizing preferred surface formats. currently they're hardcoded in the helper
    // query the surface/device and set formats, extent, present mode, based on device/surface capabilities
    const auto chosen_surface_format = choose_surface_format_(surface_formats);
    surface_format_ = chosen_surface_format.format;
    color_space_ = chosen_surface_format.colorSpace;
    depth_format_ = choose_depth_format_(device_.physical_device());
    present_mode_ = choose_present_mode_(modes);
    extent_ = choose_extent_(capabilities);
    const auto queue_indices =
        std::vector<std::uint32_t>{device_.graphics_queue_family_index(), device_.present_queue_family_index()};

    // determine how many swapchain images we are allowed
    std::uint32_t image_count = capabilities.minImageCount + 1u;
    std::uint32_t max_image_count = capabilities.maxImageCount;
    if (max_image_count > 0)
    {
        image_count = image_count < max_image_count ? image_count : max_image_count;
    }

    // TODO consider making swapchain creation a separate function
    // begin creating swapchain
    auto swapchain_create_info = ::vk::SwapchainCreateInfoKHR{};
    swapchain_create_info.surface = *surface_.get();
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = surface_format_;
    swapchain_create_info.imageColorSpace = color_space_;
    swapchain_create_info.imageExtent = extent_;
    swapchain_create_info.imageArrayLayers = 1u;
    swapchain_create_info.imageUsage =
        ::vk::ImageUsageFlagBits::eColorAttachment | ::vk::ImageUsageFlagBits::eTransferDst;
    if (device_.graphics_queue_family_index() == device_.present_queue_family_index())
    {
        swapchain_create_info.imageSharingMode = ::vk::SharingMode::eExclusive;
    }
    else
    {
        swapchain_create_info.imageSharingMode = ::vk::SharingMode::eConcurrent;
        swapchain_create_info.queueFamilyIndexCount = 2u;
        swapchain_create_info.pQueueFamilyIndices = queue_indices.data();
    }
    swapchain_create_info.preTransform = capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = ::vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapchain_create_info.presentMode = present_mode_;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = *swapchain_ != VK_NULL_HANDLE ? *swapchain_ : ::vk::SwapchainKHR{};
    swapchain_ = ::vk::raii::SwapchainKHR(device_.native_handle(), swapchain_create_info);

    // get our images from the swapchain and create image_views for each image
    images_ = swapchain_.getImages();
    image_views_.clear();
    image_views_.reserve(images_.size());
    for (const auto &image : images_)
    {
        auto image_view_create_info = ::vk::ImageViewCreateInfo{};
        image_view_create_info.image = image;
        image_view_create_info.viewType = ::vk::ImageViewType::e2D;
        image_view_create_info.format = surface_format_;
        image_view_create_info.components = ::vk::ComponentMapping{
            ::vk::ComponentSwizzle::eIdentity,
            ::vk::ComponentSwizzle::eIdentity,
            ::vk::ComponentSwizzle::eIdentity,
            ::vk::ComponentSwizzle::eIdentity,
        };
        image_view_create_info.subresourceRange.aspectMask = ::vk::ImageAspectFlagBits::eColor;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;

        image_views_.emplace_back(device_.native_handle(), image_view_create_info);
    }

    // TODO consider making depth image creation its own function
    // create depth image
    auto depth_image_create_info = ::vk::ImageCreateInfo{};
    depth_image_create_info.sType = ::vk::StructureType::eImageCreateInfo;
    depth_image_create_info.pNext = nullptr;
    depth_image_create_info.flags = {};
    depth_image_create_info.imageType = ::vk::ImageType::e2D;
    depth_image_create_info.format = depth_format_;
    depth_image_create_info.extent = ::vk::Extent3D{extent_.width, extent_.height, 1u};
    depth_image_create_info.mipLevels = 1u;
    depth_image_create_info.arrayLayers = 1u;
    depth_image_create_info.samples = ::vk::SampleCountFlagBits::e1;
    depth_image_create_info.tiling = ::vk::ImageTiling::eOptimal;
    depth_image_create_info.usage = ::vk::ImageUsageFlagBits::eDepthStencilAttachment;
    depth_image_create_info.sharingMode = ::vk::SharingMode::eExclusive;
    depth_image_create_info.queueFamilyIndexCount = 0u;
    depth_image_create_info.pQueueFamilyIndices = nullptr;
    depth_image_create_info.initialLayout = ::vk::ImageLayout::eUndefined;
    depth_image_ = ::vk::raii::Image(device_.native_handle(), depth_image_create_info);

    // create memory for depth image
    auto depth_memory_index = device_.find_memory_type_index(
        depth_image_.getMemoryRequirements(), ::vk::MemoryPropertyFlagBits::eDeviceLocal);
    auto depth_memory_allocate_info = ::vk::MemoryAllocateInfo{};
    depth_memory_allocate_info.sType = ::vk::StructureType::eMemoryAllocateInfo;
    depth_memory_allocate_info.pNext = nullptr;
    depth_memory_allocate_info.allocationSize = depth_image_.getMemoryRequirements().size;
    depth_memory_allocate_info.memoryTypeIndex = depth_memory_index;
    depth_image_memory_ = ::vk::raii::DeviceMemory(device_.native_handle(), depth_memory_allocate_info);

    // bind image to memory
    depth_image_.bindMemory(*depth_image_memory_, 0);

    // create image view for depth image
    auto depth_image_view_create_info = ::vk::ImageViewCreateInfo{};
    depth_image_view_create_info.sType = ::vk::StructureType::eImageViewCreateInfo;
    depth_image_view_create_info.pNext = nullptr;
    depth_image_view_create_info.flags = {};
    depth_image_view_create_info.image = *depth_image_;
    depth_image_view_create_info.viewType = ::vk::ImageViewType::e2D;
    depth_image_view_create_info.format = depth_format_;
    depth_image_view_create_info.components = ::vk::ComponentMapping{
        ::vk::ComponentSwizzle::eIdentity,
        ::vk::ComponentSwizzle::eIdentity,
        ::vk::ComponentSwizzle::eIdentity,
        ::vk::ComponentSwizzle::eIdentity,
    };
    depth_image_view_create_info.subresourceRange.aspectMask = ::vk::ImageAspectFlagBits::eDepth;
    depth_image_view_create_info.subresourceRange.baseMipLevel = 0u;
    depth_image_view_create_info.subresourceRange.levelCount = 1u;
    depth_image_view_create_info.subresourceRange.baseArrayLayer = 0u;
    depth_image_view_create_info.subresourceRange.layerCount = 1u;
    depth_image_view_ = ::vk::raii::ImageView(device_.native_handle(), depth_image_view_create_info);
}

auto VulkanSwapchain::destroy_() -> void
{
    image_views_.clear();
    images_.clear();
    depth_image_view_.clear();
    depth_image_memory_.clear();
    depth_image_.clear();
}

auto VulkanSwapchain::choose_surface_format_(std::span<const ::vk::SurfaceFormatKHR> formats) -> ::vk::SurfaceFormatKHR
{
    // TODO: Consider parameterizing the preferred format
    const auto preferred_format =
        ::vk::SurfaceFormatKHR{::vk::Format::eB8G8R8A8Srgb, ::vk::ColorSpaceKHR::eSrgbNonlinear};
    if (std::ranges::find(formats, preferred_format, &::vk::SurfaceFormatKHR::format) != formats.end())
    {
        for (const auto &fmt : formats)
        {
            if (fmt.format == preferred_format.format && fmt.colorSpace == preferred_format.colorSpace)
            {
                // found preferred format, use it
                return fmt;
            }
        }
    }

    for (const auto &fmt : formats)
    {
        if (fmt.format != ::vk::Format::eUndefined)
        {
            // preferred not found, use first defined format
            return fmt;
        }
    }

    arm::log::warn("Swapchain format undefined. This shouldn't happen");
    return formats.front();
}

auto VulkanSwapchain::choose_present_mode_(std::span<const ::vk::PresentModeKHR> modes) -> ::vk::PresentModeKHR
{
    if (std::ranges::find(modes, ::vk::PresentModeKHR::eMailbox) != modes.end())
    {
        return ::vk::PresentModeKHR::eMailbox;
    }
    return ::vk::PresentModeKHR::eFifo;
}

auto VulkanSwapchain::choose_extent_(const ::vk::SurfaceCapabilitiesKHR &capabilities) -> ::vk::Extent2D
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }

    arm::log::error("currentExtent was not defined, should not get here");
    return {800, 600};
}

auto VulkanSwapchain::choose_depth_format_(const ::vk::raii::PhysicalDevice &physical_device) -> ::vk::Format
{
    // TODO Consider parameterizing preferred depth formats
    auto preferred = std::vector{
        ::vk::Format::eD32SfloatS8Uint,
        ::vk::Format::eD24UnormS8Uint,
        ::vk::Format::eD32Sfloat,
    };

    for (const auto &entry : preferred)
    {
        auto format_properties = physical_device.getFormatProperties(entry);
        if (format_properties.optimalTilingFeatures & ::vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        {
            return entry;
        }
    }
    arm::log::warn("Depth format undefined. This shouldn't happen.");
    return ::vk::Format::eUndefined;
}

} // namespace pong
