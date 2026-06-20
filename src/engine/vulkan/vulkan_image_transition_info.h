#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace pong
{

struct VulkanImageTransitionInfo
{
    ::vk::ImageLayout src_layout;
    ::vk::ImageLayout dst_layout;
    ::vk::AccessFlags2 src_access;
    ::vk::AccessFlags2 dst_access;
    ::vk::PipelineStageFlags2 src_stage;
    ::vk::PipelineStageFlags2 dst_stage;

    // swapchain image before color attachment
    static auto undef_to_color_optimal() -> VulkanImageTransitionInfo
    {
        return VulkanImageTransitionInfo{
            .src_layout = ::vk::ImageLayout::eUndefined,
            .dst_layout = ::vk::ImageLayout::eColorAttachmentOptimal,
            .src_access = {},
            .dst_access = ::vk::AccessFlagBits2::eColorAttachmentWrite,
            .src_stage = ::vk::PipelineStageFlagBits2::eTopOfPipe,
            .dst_stage = ::vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        };
    }

    // swapchain image after color attachment
    static auto color_optimal_to_present() -> VulkanImageTransitionInfo
    {
        return VulkanImageTransitionInfo{
            .src_layout = ::vk::ImageLayout::eColorAttachmentOptimal,
            .dst_layout = ::vk::ImageLayout::ePresentSrcKHR,
            .src_access = ::vk::AccessFlagBits2::eColorAttachmentWrite,
            .dst_access = {},
            .src_stage = ::vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .dst_stage = ::vk::PipelineStageFlagBits2::eBottomOfPipe,
        };
    };

    // depth
    static auto undef_to_depth_optimal() -> VulkanImageTransitionInfo
    {
        return VulkanImageTransitionInfo{
            .src_layout = ::vk::ImageLayout::eUndefined,
            .dst_layout = ::vk::ImageLayout::eDepthStencilAttachmentOptimal,
            .src_access = {},
            .dst_access = ::vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
            .src_stage = ::vk::PipelineStageFlagBits2::eTopOfPipe,
            .dst_stage = ::vk::PipelineStageFlagBits2::eEarlyFragmentTests,
        };
    }

    // before texture upload for fragment shader sampling
    static auto undef_to_transfer_dst_optimal() -> VulkanImageTransitionInfo
    {
        return VulkanImageTransitionInfo{
            .src_layout = ::vk::ImageLayout::eUndefined,
            .dst_layout = ::vk::ImageLayout::eTransferDstOptimal,
            .src_access = {},
            .dst_access = ::vk::AccessFlagBits2::eTransferWrite,
            .src_stage = ::vk::PipelineStageFlagBits2::eTopOfPipe,
            .dst_stage = ::vk::PipelineStageFlagBits2::eTransfer,
        };
    }

    // after texture upload for fragment shader sampling
    static auto transfer_dst_optimal_to_shader_rd_only_optimal() -> VulkanImageTransitionInfo
    {
        return VulkanImageTransitionInfo{
            .src_layout = ::vk::ImageLayout::eTransferDstOptimal,
            .dst_layout = ::vk::ImageLayout::eShaderReadOnlyOptimal,
            .src_access = ::vk::AccessFlagBits2::eTransferWrite,
            .dst_access = ::vk::AccessFlagBits2::eShaderSampledRead,
            .src_stage = ::vk::PipelineStageFlagBits2::eTransfer,
            .dst_stage = ::vk::PipelineStageFlagBits2::eFragmentShader,
        };
    }
}; // struct VulkanImageTransitionInfo

} // namespace pong
