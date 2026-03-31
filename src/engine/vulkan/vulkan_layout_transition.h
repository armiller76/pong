#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace pong
{

struct transition_info
{
    ::vk::ImageLayout src_layout;
    ::vk::ImageLayout dst_layout;
    ::vk::AccessFlags2 src_access;
    ::vk::AccessFlags2 dst_access;
    ::vk::PipelineStageFlags2 src_stage;
    ::vk::PipelineStageFlags2 dst_stage;

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
};

}
