#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace pong
{

struct VulkanPipelineResources
{
    ::vk::raii::PipelineLayout layout;
    ::vk::raii::Pipeline pipeline;
    ::vk::raii::DescriptorSetLayout ubo_descriptor_set_layout;
    ::vk::raii::DescriptorSetLayout texture_descriptor_set_layout;
}; // struct VulkanPipelineResources

struct transition_info
{
    ::vk::ImageLayout src_layout;
    ::vk::ImageLayout dst_layout;
    ::vk::AccessFlags2 src_access;
    ::vk::AccessFlags2 dst_access;
    ::vk::PipelineStageFlags2 src_stage;
    ::vk::PipelineStageFlags2 dst_stage;

    // swapchain image before color attachment
    static auto undef_to_color_optimal() -> transition_info
    {
        return transition_info{
            .src_layout = ::vk::ImageLayout::eUndefined,
            .dst_layout = ::vk::ImageLayout::eColorAttachmentOptimal,
            .src_access = {},
            .dst_access = ::vk::AccessFlagBits2::eColorAttachmentWrite,
            .src_stage = ::vk::PipelineStageFlagBits2::eTopOfPipe,
            .dst_stage = ::vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        };
    }

    // swapchain image after color attachment
    static auto color_optimal_to_present() -> transition_info
    {
        return transition_info{
            .src_layout = ::vk::ImageLayout::eColorAttachmentOptimal,
            .dst_layout = ::vk::ImageLayout::ePresentSrcKHR,
            .src_access = ::vk::AccessFlagBits2::eColorAttachmentWrite,
            .dst_access = {},
            .src_stage = ::vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .dst_stage = ::vk::PipelineStageFlagBits2::eBottomOfPipe,
        };
    };

    // depth
    static auto undef_to_depth_optimal() -> transition_info
    {
        return transition_info{
            .src_layout = ::vk::ImageLayout::eUndefined,
            .dst_layout = ::vk::ImageLayout::eDepthStencilAttachmentOptimal,
            .src_access = {},
            .dst_access = ::vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
            .src_stage = ::vk::PipelineStageFlagBits2::eTopOfPipe,
            .dst_stage = ::vk::PipelineStageFlagBits2::eEarlyFragmentTests,
        };
    }

    // before texture upload for fragment shader sampling
    static auto undef_to_transfer_dst_optimal() -> transition_info
    {
        return transition_info{
            .src_layout = ::vk::ImageLayout::eUndefined,
            .dst_layout = ::vk::ImageLayout::eTransferDstOptimal,
            .src_access = {},
            .dst_access = ::vk::AccessFlagBits2::eTransferWrite,
            .src_stage = ::vk::PipelineStageFlagBits2::eTopOfPipe,
            .dst_stage = ::vk::PipelineStageFlagBits2::eTransfer,
        };
    }

    // after texture upload for fragment shader sampling
    static auto transfer_dst_optimal_to_shader_rd_only_optimal() -> transition_info
    {
        return transition_info{
            .src_layout = ::vk::ImageLayout::eTransferDstOptimal,
            .dst_layout = ::vk::ImageLayout::eShaderReadOnlyOptimal,
            .src_access = ::vk::AccessFlagBits2::eTransferWrite,
            .dst_access = ::vk::AccessFlagBits2::eShaderSampledRead,
            .src_stage = ::vk::PipelineStageFlagBits2::eTransfer,
            .dst_stage = ::vk::PipelineStageFlagBits2::eFragmentShader,
        };
    }
};

inline auto transition(
    ::vk::raii::CommandBuffer &command_buffer,
    ::vk::Image image,
    ::vk::ImageAspectFlags aspect_flags,
    transition_info info) -> void
{
    auto barrier = ::vk::ImageMemoryBarrier2{};
    barrier.sType = ::vk::StructureType::eImageMemoryBarrier2;
    barrier.srcStageMask = info.src_stage;
    barrier.srcAccessMask = info.src_access;
    barrier.dstStageMask = info.dst_stage;
    barrier.dstAccessMask = info.dst_access;
    barrier.oldLayout = info.src_layout;
    barrier.newLayout = info.dst_layout;
    // TODO when do we stop ignoring queue family index?
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    barrier.subresourceRange.aspectMask = aspect_flags;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    auto dependency_info = ::vk::DependencyInfo{};
    dependency_info.dependencyFlags = {};
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &barrier;

    command_buffer.pipelineBarrier2(dependency_info);
}

inline auto make_color_attachment(::vk::ImageView image_view, ::vk::ClearColorValue clear_color)
    -> ::vk::RenderingAttachmentInfo
{
    auto result = ::vk::RenderingAttachmentInfo{};
    result.sType = ::vk::StructureType::eRenderingAttachmentInfo;
    result.pNext = nullptr;
    result.imageView = image_view;
    result.imageLayout = ::vk::ImageLayout::eColorAttachmentOptimal;
    result.loadOp = ::vk::AttachmentLoadOp::eClear;
    result.storeOp = ::vk::AttachmentStoreOp::eStore;
    result.clearValue = clear_color;
    result.resolveMode = ::vk::ResolveModeFlagBits::eNone;
    result.resolveImageView = nullptr;
    result.resolveImageLayout = ::vk::ImageLayout::eUndefined;
    return result;
}

inline auto make_depth_attachment(::vk::ImageView image_view) -> ::vk::RenderingAttachmentInfo
{
    auto result = ::vk::RenderingAttachmentInfo{};
    result.sType = ::vk::StructureType::eRenderingAttachmentInfo;
    result.pNext = nullptr;
    result.imageView = image_view;
    result.imageLayout = ::vk::ImageLayout::eDepthStencilAttachmentOptimal;
    result.loadOp = ::vk::AttachmentLoadOp::eClear;
    result.storeOp = ::vk::AttachmentStoreOp::eDontCare;
    result.clearValue = ::vk::ClearDepthStencilValue{1.0f, 0};
    result.resolveMode = ::vk::ResolveModeFlagBits::eNone;
    result.resolveImageView = nullptr;
    result.resolveImageLayout = ::vk::ImageLayout::eUndefined;
    return result;
}

} // namespace pong
