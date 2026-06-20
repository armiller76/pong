#include "vulkan_gpu_image.h"

#include <cstddef>
#include <utility>

#include <vulkan/vulkan_raii.hpp>

#include "engine/engine_types.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_gpu_buffer.h"
#include "engine/vulkan/vulkan_image_transition_info.h"
#include "engine/vulkan/vulkan_immediate_command_context.h"
#include "engine/vulkan/vulkan_render_utils.h"
#include "engine/vulkan/vulkan_utils.h"
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
    , sampler_{sampler}
    , image_{nullptr}
    , memory_{nullptr}
    , view_{nullptr}
{
    arm::log::debug("VulkanGpuImage Constructor");

    const auto image_create_info = ::vk::ImageCreateInfo{
        .sType = ::vk::StructureType::eImageCreateInfo,
        .pNext = nullptr,
        .flags = {},
        .imageType = ::vk::ImageType::e2D,
        .format = format_,
        .extent =
            {
                .width = extent_.width,
                .height = extent_.height,
                .depth = 1u,
            },
        .mipLevels = 1u,
        .arrayLayers = 1u,
        .samples = ::vk::SampleCountFlagBits::e1,
        .tiling = ::vk::ImageTiling::eOptimal,
        .usage = ::vk::ImageUsageFlagBits::eTransferDst | ::vk::ImageUsageFlagBits::eSampled,
        .sharingMode = ::vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0u,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = ::vk::ImageLayout::eUndefined,
    };
    auto image_memory_pair = device_->allocate_image(image_create_info, ::vk::MemoryPropertyFlagBits::eDeviceLocal);
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
                .aspectMask = ::vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0u,
                .levelCount = 1u,
                .baseArrayLayer = 0u,
                .layerCount = 1u,
            },
    };
    auto view_result = check_vk_expected(device_->get().createImageView(view_create_info));
    if (!view_result.has_value())
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
    const auto image_size_bytes =
        std::size_t(image.extent().width) * image.extent().height * bytes_per_pixel(image.format());
    auto &cb = command_context.command_buffer();
    auto staging_buffer = VulkanGpuBuffer(
        *device_,
        ::vk::DeviceSize(image_size_bytes),
        ::vk::BufferUsageFlagBits::eTransferSrc,
        ::vk::MemoryPropertyFlagBits::eHostCoherent | ::vk::MemoryPropertyFlagBits::eHostVisible);
    staging_buffer.upload(image.pixels().data(), image.pixels().size());

    const auto begin_info = ::vk::CommandBufferBeginInfo{
        .sType = ::vk::StructureType::eCommandBufferBeginInfo,
        .pNext = nullptr,
        .flags = ::vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    };
    cb.begin(begin_info);

    transition(
        cb, image_, ::vk::ImageAspectFlagBits::eColor, VulkanImageTransitionInfo::undef_to_transfer_dst_optimal());

    const auto image_info = ::vk::BufferImageCopy2{
        .sType = ::vk::StructureType::eBufferImageCopy2,
        .pNext = nullptr,
        .bufferOffset = 0,
        .bufferRowLength = image.extent().width,
        .bufferImageHeight = image.extent().height,
        .imageSubresource =
            {
                .aspectMask = ::vk::ImageAspectFlagBits::eColor,
                .mipLevel = 0u,
                .baseArrayLayer = 0u,
                .layerCount = 1u,
            },
        .imageOffset =
            ::vk::Offset3D{
                .x = 0,
                .y = 0,
                .z = 0,
            },
        .imageExtent =
            ::vk::Extent3D{
                .width = image.extent().width,
                .height = image.extent().height,
                .depth = 1,
            },
    };
    const auto image_info_array = std::array{
        image_info,
    };
    const auto copy_info = ::vk::CopyBufferToImageInfo2{
        .sType = ::vk::StructureType::eCopyBufferToImageInfo2,
        .pNext = nullptr,
        .srcBuffer = staging_buffer.native_handle(),
        .dstImage = *image_,
        .dstImageLayout = ::vk::ImageLayout::eTransferDstOptimal,
        .regionCount = static_cast<std::uint32_t>(image_info_array.size()),
        .pRegions = image_info_array.data(),
    };

    cb.copyBufferToImage2(copy_info);
    transition(
        cb,
        image_,
        ::vk::ImageAspectFlagBits::eColor,
        VulkanImageTransitionInfo::transfer_dst_optimal_to_shader_rd_only_optimal());
    cb.end();

    const auto cb_submit_info = ::vk::CommandBufferSubmitInfo{
        .sType = ::vk::StructureType::eCommandBufferSubmitInfo,
        .pNext = nullptr,
        .commandBuffer = *cb,
        .deviceMask = 0,
    };
    const auto cb_submit_array = std::array{
        cb_submit_info,
    };
    const auto submit_info = ::vk::SubmitInfo2{
        .sType = ::vk::StructureType::eSubmitInfo2,
        .pNext = nullptr,
        .flags = {},
        .waitSemaphoreInfoCount = 0u,
        .pWaitSemaphoreInfos = nullptr,
        .commandBufferInfoCount = static_cast<std::uint32_t>(cb_submit_array.size()),
        .pCommandBufferInfos = cb_submit_array.data(),
        .signalSemaphoreInfoCount = 0u,
        .pSignalSemaphoreInfos = nullptr,
    };

    auto result = check_vk_result(device_->graphics_queue().submit2(1u, &submit_info, command_context.fence()));
    if (result.code != ResultCode::Ok)
    {
        throw arm::Exception("[{}] {}", result.code, result.message);
    }
    command_context.wait_for_fence();
}

}
