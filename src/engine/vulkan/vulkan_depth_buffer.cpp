#include "vulkan_depth_buffer.h"

#include <vulkan/vulkan_raii.hpp>

#include "utils/log.h"
#include "vulkan_device.h"

namespace pong
{

DepthBuffer::DepthBuffer(const VulkanDevice &device, ::vk::Extent2D extent)
    : device_{&device}
    , format_{device_->choose_depth_format()}
    , image_{nullptr}
    , memory_{nullptr}
    , view_{nullptr}
{
    auto create_info = ::vk::ImageCreateInfo{};
    create_info.sType = ::vk::StructureType::eImageCreateInfo;
    create_info.pNext = nullptr;
    create_info.flags = {};
    create_info.imageType = ::vk::ImageType::e2D;
    create_info.format = format_;
    create_info.extent = ::vk::Extent3D{extent.width, extent.height, 1u};
    create_info.mipLevels = 1u;
    create_info.arrayLayers = 1u;
    create_info.samples = ::vk::SampleCountFlagBits::e1;
    create_info.tiling = ::vk::ImageTiling::eOptimal;
    create_info.usage = ::vk::ImageUsageFlagBits::eDepthStencilAttachment;
    create_info.sharingMode = ::vk::SharingMode::eExclusive;
    create_info.queueFamilyIndexCount = 0u;
    create_info.pQueueFamilyIndices = nullptr;
    create_info.initialLayout = ::vk::ImageLayout::eUndefined;
    auto image_memory_pair = device_->allocate_image(create_info, ::vk::MemoryPropertyFlagBits::eDeviceLocal);
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
    view_ = ::vk::raii::ImageView(device_->native_handle(), view_create_info);
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
