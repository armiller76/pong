#include "vulkan_depth_buffer.h"

#include <utility>

#include <vulkan/vulkan_raii.hpp>

#include "engine/engine_error.h"
#include "engine/vulkan/vulkan_device.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

DepthBuffer::DepthBuffer(const VulkanDevice &device, ::vk::Extent2D extent)
    : device_{&device}
    , format_{device_->choose_depth_format()}
    , image_{nullptr}
    , memory_{nullptr}
    , view_{nullptr}
{
    arm::log::debug("DepthBuffer constructor");

    auto image_create_info = ::vk::ImageCreateInfo{};
    image_create_info.sType = ::vk::StructureType::eImageCreateInfo;
    image_create_info.pNext = nullptr;
    image_create_info.flags = {};
    image_create_info.imageType = ::vk::ImageType::e2D;
    image_create_info.format = format_;
    image_create_info.extent = ::vk::Extent3D{extent.width, extent.height, 1u};
    image_create_info.mipLevels = 1u;
    image_create_info.arrayLayers = 1u;
    image_create_info.samples = ::vk::SampleCountFlagBits::e1;
    image_create_info.tiling = ::vk::ImageTiling::eOptimal;
    image_create_info.usage = ::vk::ImageUsageFlagBits::eDepthStencilAttachment;
    image_create_info.sharingMode = ::vk::SharingMode::eExclusive;
    image_create_info.queueFamilyIndexCount = 0u;
    image_create_info.pQueueFamilyIndices = nullptr;
    image_create_info.initialLayout = ::vk::ImageLayout::eUndefined;
    auto image_memory_pair = device_->allocate_image(image_create_info, ::vk::MemoryPropertyFlagBits::eDeviceLocal);
    image_ = std::move(image_memory_pair.first);
    memory_ = std::move(image_memory_pair.second);

    auto view_create_info = ::vk::ImageViewCreateInfo{};
    view_create_info.sType = ::vk::StructureType::eImageViewCreateInfo;
    view_create_info.pNext = nullptr;
    view_create_info.flags = {};
    view_create_info.image = *image_;
    view_create_info.viewType = ::vk::ImageViewType::e2D;
    view_create_info.format = format_;
    view_create_info.components = ::vk::ComponentMapping{
        ::vk::ComponentSwizzle::eIdentity,
        ::vk::ComponentSwizzle::eIdentity,
        ::vk::ComponentSwizzle::eIdentity,
        ::vk::ComponentSwizzle::eIdentity,
    };
    view_create_info.subresourceRange.aspectMask = ::vk::ImageAspectFlagBits::eDepth;
    view_create_info.subresourceRange.baseMipLevel = 0u;
    view_create_info.subresourceRange.levelCount = 1u;
    view_create_info.subresourceRange.baseArrayLayer = 0u;
    view_create_info.subresourceRange.layerCount = 1u;
    auto view_result = check_vk_expected(device_->native_handle().createImageView(view_create_info));
    if (!view_result)
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
