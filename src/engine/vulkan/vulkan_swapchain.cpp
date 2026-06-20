#include "vulkan_swapchain.h"

#include <algorithm>
#include <cstdint>
#include <format>
#include <utility>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_surface.h"
#include "engine/vulkan/vulkan_utils.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

VulkanSwapchain::VulkanSwapchain(const VulkanDevice &device, const VulkanSurface &surface)
    : device_{device}
    , surface_{surface}
    , swapchain_{nullptr}
{
    arm::log::debug("VulkanSwapchain constructor");
    init_();
    create_();
}

auto VulkanSwapchain::recreate() -> void
{
    destroy_();
    init_();
    create_();
}

auto VulkanSwapchain::get() const -> const ::vk::raii::SwapchainKHR &
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

auto VulkanSwapchain::semaphores() const -> const std::vector<::vk::raii::Semaphore> &
{
    return render_finished_semaphores_;
}

auto VulkanSwapchain::init_() -> void
{
    capabilities_ = device_.get_physical_device().getSurfaceCapabilitiesKHR(surface_.native_handle());
    formats_ = device_.get_physical_device().getSurfaceFormatsKHR(surface_.native_handle());
    modes_ = device_.get_physical_device().getSurfacePresentModesKHR(surface_.native_handle());
    // if the device didn't give us any surface formats or present modes, crash
    if (formats_.empty() || modes_.empty())
    {
        throw arm::Exception("No surface formats or present modes or both");
    }
    // TODO consider parameterizing preferred surface formats. currently they're hardcoded in the helper
    // query the surface/device and set formats, extent, present mode, based on device/surface capabilities
    const auto chosen_surface_format = choose_surface_format_(formats_);
    surface_format_ = chosen_surface_format.format;
    color_space_ = chosen_surface_format.colorSpace;
    present_mode_ = choose_present_mode_(modes_);
    extent_ = choose_extent_(capabilities_);
}

auto VulkanSwapchain::create_() -> void
{
    const auto queue_indices =
        std::vector<std::uint32_t>{device_.graphics_queue_family_index(), device_.present_queue_family_index()};

    // determine how many swapchain images we are allowed
    std::uint32_t image_count = capabilities_.minImageCount + 1u;
    if (const std::uint32_t max_image_count = capabilities_.maxImageCount; max_image_count > 0)
    {
        image_count = image_count < max_image_count ? image_count : max_image_count;
    }

    // begin creating swapchain
    auto swapchain_create_info = ::vk::SwapchainCreateInfoKHR{
        .sType = ::vk::StructureType::eSwapchainCreateInfoKHR,
        .surface = surface_.native_handle(),
        .minImageCount = image_count,
        .imageFormat = surface_format_,
        .imageColorSpace = color_space_,
        .imageExtent = extent_,
        .imageArrayLayers = 1u,
        .imageUsage = ::vk::ImageUsageFlagBits::eColorAttachment | ::vk::ImageUsageFlagBits::eTransferDst,
        .preTransform = capabilities_.currentTransform,
        .compositeAlpha = ::vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = present_mode_,
        .clipped = VK_TRUE,
        .oldSwapchain = *swapchain_ != VK_NULL_HANDLE ? *swapchain_ : ::vk::SwapchainKHR{},
    };

    if (device_.graphics_queue_family_index() == device_.present_queue_family_index())
    {
        swapchain_create_info.imageSharingMode = ::vk::SharingMode::eExclusive;
        swapchain_create_info.queueFamilyIndexCount = 0u;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
    }
    else
    {
        swapchain_create_info.imageSharingMode = ::vk::SharingMode::eConcurrent;
        swapchain_create_info.queueFamilyIndexCount = static_cast<std::uint32_t>(queue_indices.size());
        swapchain_create_info.pQueueFamilyIndices = queue_indices.data();
    }

    auto swapchain_result = check_vk_expected(device_.get().createSwapchainKHR(swapchain_create_info));
    if (!swapchain_result.has_value())
    {
        throw arm::Exception("uanble to create swapchain");
    }
    swapchain_ = std::move(swapchain_result.value());

    // get the created images from the swapchain and create image_views for each
    images_ = swapchain_.getImages();

    image_views_.clear();
    image_views_.reserve(images_.size());
    for (const auto &image : images_)
    {
        const auto image_view_create_info = ::vk::ImageViewCreateInfo{
            .sType = ::vk::StructureType::eImageViewCreateInfo,
            .pNext = nullptr,
            .flags = {},
            .image = image,
            .viewType = ::vk::ImageViewType::e2D,
            .format = surface_format_,
            .components{
                .r = ::vk::ComponentSwizzle::eIdentity,
                .g = ::vk::ComponentSwizzle::eIdentity,
                .b = ::vk::ComponentSwizzle::eIdentity,
                .a = ::vk::ComponentSwizzle::eIdentity,
            },
            .subresourceRange{
                .aspectMask = ::vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        auto view_result = check_vk_expected(device_.get().createImageView(image_view_create_info));
        if (!view_result)
        {
            throw arm::Exception("unable to create image view");
        }
        image_views_.push_back(std::move(view_result.value()));
    }

    for (std::size_t i = 0; i < images_.size(); ++i)
    {
        auto semaphore_result = check_vk_expected(device_.get().createSemaphore(::vk::SemaphoreCreateInfo{}));
        if (!semaphore_result.has_value())
        {
            throw arm::Exception("unable to create semaphore");
        }

#ifndef NDEBUG
        const auto debug_name_str = std::format("Render Finished Semaphore {}", i);
        const auto debug_name_info = ::vk::DebugUtilsObjectNameInfoEXT{
            .sType = ::vk::StructureType::eDebugUtilsObjectNameInfoEXT,
            .pNext = nullptr,
            .objectType = ::vk::ObjectType::eSemaphore,
            .objectHandle = reinterpret_cast<std::uint64_t>(static_cast<::VkSemaphore>(*semaphore_result.value())),
            .pObjectName = debug_name_str.c_str(),
        };
        device_.get().setDebugUtilsObjectNameEXT(debug_name_info);
#endif

        render_finished_semaphores_.push_back(std::move(semaphore_result.value()));
    }
}

auto VulkanSwapchain::destroy_() -> void
{
    render_finished_semaphores_.clear();
    image_views_.clear();
    images_.clear();
}

auto VulkanSwapchain::choose_surface_format_(std::span<const ::vk::SurfaceFormatKHR> formats) -> ::vk::SurfaceFormatKHR
{
    // TODO: Consider parameterizing the preferred format
    const auto preferred_format = ::vk::SurfaceFormatKHR{
        .format = ::vk::Format::eB8G8R8A8Srgb,
        .colorSpace = ::vk::ColorSpaceKHR::eSrgbNonlinear,
    };

    if (const auto found_preferred = std::ranges::find(formats, preferred_format); found_preferred != formats.end())
    {
        return *found_preferred;
    }

    for (const auto &format : formats)
    {
        if (format.format != ::vk::Format::eUndefined)
        {
            return format;
        }
    }

    arm::log::warn("Swapchain format undefined/empty. Falling back to first available");
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

    return {
        .width = 800,
        .height = 600,
    };
}

} // namespace pong
