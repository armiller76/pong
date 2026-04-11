#include "vulkan_gpu_image.h"

#include <vulkan/vulkan_raii.hpp>

#include "utils/log.h"
#include "vulkan_device.h"

namespace pong
{

GpuImage::GpuImage(const VulkanDevice &device, ::vk::Extent2D extent, ::vk::Format format)
    : device_{&device}
    , image_{nullptr}
    , memory_{nullptr}
    , view_{nullptr}
    , sampler_{nullptr}
    , extent_{extent}
    , format_{format}
{
    arm::log::debug("GpuImage Constructor");

    auto image_create_info = ::vk::ImageCreateInfo{};
    image_create_info.sType = ::vk::StructureType::eImageCreateInfo;
    image_create_info.pNext = nullptr;
    image_create_info.flags = {};
    image_create_info.imageType = ::vk::ImageType::e2D;
    image_create_info.format = format_;
    image_create_info.extent = ::vk::Extent3D{extent_.width, extent_.height, 1u};
    image_create_info.mipLevels = 1u;
    image_create_info.arrayLayers = 1u;
    image_create_info.samples = ::vk::SampleCountFlagBits::e1;
    image_create_info.tiling = ::vk::ImageTiling::eOptimal;
    image_create_info.usage = ::vk::ImageUsageFlagBits::eTransferDst | ::vk::ImageUsageFlagBits::eSampled;
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
    view_create_info.subresourceRange.aspectMask = ::vk::ImageAspectFlagBits::eColor;
    view_create_info.subresourceRange.baseMipLevel = 0u;
    view_create_info.subresourceRange.levelCount = 1u;
    view_create_info.subresourceRange.baseArrayLayer = 0u;
    view_create_info.subresourceRange.layerCount = 1u;
    view_ = ::vk::raii::ImageView(device_->native_handle(), view_create_info);

    auto sampler_create_info = ::vk::SamplerCreateInfo{};
    sampler_create_info.sType = ::vk::StructureType::eSamplerCreateInfo;
    sampler_create_info.pNext = nullptr;
    sampler_create_info.flags = {};
    sampler_create_info.magFilter = ::vk::Filter::eLinear;
    sampler_create_info.minFilter = ::vk::Filter::eLinear;
    sampler_create_info.mipmapMode = ::vk::SamplerMipmapMode::eLinear;
    sampler_create_info.addressModeU = ::vk::SamplerAddressMode::eClampToEdge;
    sampler_create_info.addressModeV = ::vk::SamplerAddressMode::eClampToEdge;
    sampler_create_info.addressModeW = ::vk::SamplerAddressMode::eClampToEdge;
    sampler_create_info.mipLodBias = 0.0f;
    sampler_create_info.anisotropyEnable = ::vk::False;
    sampler_create_info.maxAnisotropy = 1.0f;
    sampler_create_info.compareEnable = ::vk::False;
    sampler_create_info.compareOp = ::vk::CompareOp::eNever;
    sampler_create_info.minLod = 0.0f;
    sampler_create_info.maxLod = 0.0f;
    sampler_create_info.borderColor = ::vk::BorderColor::eIntOpaqueBlack;
    sampler_create_info.unnormalizedCoordinates = ::vk::False;
    sampler_ = ::vk::raii::Sampler(device_->native_handle(), sampler_create_info);
}

auto GpuImage::image() const -> ::vk::Image
{
    return *image_;
}

auto GpuImage::image_view() const -> ::vk::ImageView
{
    return *view_;
}

auto GpuImage::sampler() const -> ::vk::Sampler
{
    return *sampler_;
}

auto GpuImage::extent() const -> ::vk::Extent2D
{
    return extent_;
}

auto GpuImage::format() const -> ::vk::Format
{
    return format_;
}

}
