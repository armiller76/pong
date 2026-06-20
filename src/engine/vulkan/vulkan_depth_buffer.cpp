#include "vulkan_depth_buffer.h"

#include <utility>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_utils.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

DepthBuffer::DepthBuffer(const VulkanDevice &device, ::vk::Extent2D extent)
    : format_{device.choose_depth_format()}
    , image_{nullptr}
    , memory_{nullptr}
    , view_{nullptr}
{
    arm::log::debug("DepthBuffer constructor");

    const auto image_create_info = ::vk::ImageCreateInfo{
        .sType = ::vk::StructureType::eImageCreateInfo,
        .pNext = nullptr,
        .flags = {},
        .imageType = ::vk::ImageType::e2D,
        .format = format_,
        .extent =
            {
                .width = extent.width,
                .height = extent.height,
                .depth = 1u,
            },
        .mipLevels = 1u,
        .arrayLayers = 1u,
        .samples = ::vk::SampleCountFlagBits::e1,
        .tiling = ::vk::ImageTiling::eOptimal,
        .usage = ::vk::ImageUsageFlagBits::eDepthStencilAttachment,
        .sharingMode = ::vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0u,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = ::vk::ImageLayout::eUndefined,
    };
    auto image_memory_pair = device.allocate_image(image_create_info, ::vk::MemoryPropertyFlagBits::eDeviceLocal);

    // allocate_image will throw if it fails
    image_ = std::move(image_memory_pair.first);
    memory_ = std::move(image_memory_pair.second);

    const auto view_create_info = ::vk::ImageViewCreateInfo{
        .sType = ::vk::StructureType::eImageViewCreateInfo,
        .pNext = nullptr,
        .flags = {},
        .image = *image_,
        .viewType = ::vk::ImageViewType::e2D,
        .format = format_,
        .components =
            {
                .r = ::vk::ComponentSwizzle::eIdentity,
                .g = ::vk::ComponentSwizzle::eIdentity,
                .b = ::vk::ComponentSwizzle::eIdentity,
                .a = ::vk::ComponentSwizzle::eIdentity,
            },
        .subresourceRange =
            {
                .aspectMask = ::vk::ImageAspectFlagBits::eDepth,
                .baseMipLevel = 0u,
                .levelCount = 1u,
                .baseArrayLayer = 0u,
                .layerCount = 1u,
            },
    };

    auto view_result = check_vk_expected(device.get().createImageView(view_create_info));
    if (!view_result.has_value())
    {
        throw arm::Exception("unable to create depth buffer image view");
    }
    view_ = std::move(view_result.value());
}

auto DepthBuffer::format() const -> ::vk::Format
{
    return format_;
}

auto DepthBuffer::image() const -> ::vk::Image
{
    return *image_;
}

auto DepthBuffer::image_view() const -> ::vk::ImageView
{
    return *view_;
}

} // namespace pong
