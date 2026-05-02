#include "vulkan_gpu_image.h"

#include <cstddef>
#include <utility>

#include <vulkan/vulkan_raii.hpp>

#include "engine/engine_error.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_gpu_buffer.h"
#include "engine/vulkan/vulkan_immediate_command_context.h"
#include "engine/vulkan/vulkan_render_utils.h"
#include "graphics/image.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

VulkanGpuImage::VulkanGpuImage(
    const VulkanDevice &device,
    ::vk::Extent2D extent,
    ::vk::Sampler sampler,
    ::vk::Format format)
    : device_{&device}
    , extent_{extent}
    , format_{format}
    , image_{nullptr}
    , memory_{nullptr}
    , view_{nullptr}
    , sampler_{sampler}
{
    arm::log::debug("VulkanGpuImage Constructor");

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
    auto view_result = check_vk_expected(device_->native_handle().createImageView(view_create_info));
    if (!view_result)
    {
        throw arm::Exception("uanble to create imageview");
    }
    view_ = std::move(view_result.value());
}

auto VulkanGpuImage::image() const -> ::vk::Image
{
    return *image_;
}

auto VulkanGpuImage::image_view() const -> ::vk::ImageView
{
    return *view_;
}

auto VulkanGpuImage::sampler() const -> ::vk::Sampler
{
    return sampler_;
}

auto VulkanGpuImage::extent() const -> ::vk::Extent2D
{
    return extent_;
}

auto VulkanGpuImage::format() const -> ::vk::Format
{
    return format_;
}

auto VulkanGpuImage::upload(VulkanImmediateCommandContext &command_context, const Image &image) -> void
{
    auto image_size_bytes = std::size_t(image.extent().width) * image.extent().height * bytes_per_pixel(image.format());
    auto &cb = command_context.command_buffer();
    auto staging_buffer = VulkanGpuBuffer(
        *device_,
        ::vk::DeviceSize(image_size_bytes),
        ::vk::BufferUsageFlagBits::eTransferSrc,
        ::vk::MemoryPropertyFlagBits::eHostCoherent | ::vk::MemoryPropertyFlagBits::eHostVisible);
    staging_buffer.upload(image.pixels().data(), image.pixels().size());

    auto begin_info = ::vk::CommandBufferBeginInfo{};
    begin_info.sType = ::vk::StructureType::eCommandBufferBeginInfo;
    begin_info.flags = ::vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    cb.begin(begin_info);
    transition(cb, image_, ::vk::ImageAspectFlagBits::eColor, transition_info::undef_to_transfer_dst_optimal());

    auto image_info = ::vk::BufferImageCopy2{};
    image_info.sType = ::vk::StructureType::eBufferImageCopy2;
    image_info.pNext = nullptr;
    image_info.bufferOffset = 0;
    image_info.bufferRowLength = image.extent().width;
    image_info.bufferImageHeight = image.extent().height;
    image_info.imageSubresource.aspectMask = ::vk::ImageAspectFlagBits::eColor;
    image_info.imageSubresource.mipLevel = 0u;
    image_info.imageSubresource.layerCount = 1u;
    image_info.imageSubresource.baseArrayLayer = 0u;
    image_info.imageOffset = ::vk::Offset3D{0, 0, 0};
    image_info.imageExtent = ::vk::Extent3D{image.extent().width, image.extent().height, 1};
    auto image_info_array = std::array{
        image_info,
    };

    auto copy_info = ::vk::CopyBufferToImageInfo2{};
    copy_info.sType = ::vk::StructureType::eCopyBufferToImageInfo2;
    copy_info.pNext = nullptr;
    copy_info.srcBuffer = staging_buffer.native_handle();
    copy_info.dstImage = *image_;
    copy_info.dstImageLayout = ::vk::ImageLayout::eTransferDstOptimal;
    copy_info.regionCount = static_cast<std::uint32_t>(image_info_array.size());
    copy_info.pRegions = image_info_array.data();

    cb.copyBufferToImage2(copy_info);
    transition(
        cb,
        image_,
        ::vk::ImageAspectFlagBits::eColor,
        transition_info::transfer_dst_optimal_to_shader_rd_only_optimal());
    cb.end();

    auto cb_submit_info = ::vk::CommandBufferSubmitInfo{};
    cb_submit_info.sType = ::vk::StructureType::eCommandBufferSubmitInfo;
    cb_submit_info.pNext = nullptr;
    cb_submit_info.commandBuffer = *cb;
    cb_submit_info.deviceMask = 0;
    auto cb_submit_array = std::array{
        cb_submit_info,
    };

    auto submit_info = ::vk::SubmitInfo2{};
    submit_info.sType = ::vk::StructureType::eSubmitInfo2;
    submit_info.pNext = nullptr;
    submit_info.flags = {};
    submit_info.waitSemaphoreInfoCount = 0u;
    submit_info.pWaitSemaphoreInfos = nullptr;
    submit_info.commandBufferInfoCount = static_cast<std::uint32_t>(cb_submit_array.size());
    submit_info.pCommandBufferInfos = cb_submit_array.data();
    submit_info.signalSemaphoreInfoCount = 0u;
    submit_info.pSignalSemaphoreInfos = nullptr;

    auto result = check_vk_result(device_->graphics_queue().submit2(1u, &submit_info, command_context.fence()));
    if (result.code != ResultCode::Ok)
    {
        throw arm::Exception("[{}] {}", result.code, result.message);
    }
    command_context.wait_for_fence();
}

}
