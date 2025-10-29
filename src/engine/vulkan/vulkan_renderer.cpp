#include "vulkan_renderer.h"

#include <array>

#include "graphics/color.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/log.h"
#include "vulkan_device.h"

namespace pong
{

VulkanRenderer::VulkanRenderer(const VulkanDevice &device, const VulkanSurface &surface, const Color clear_color)
    : device_{device}
    , surface_{surface}
    , swapchain_{device_, surface_}
    , command_context_{device_, 2}
    , graphics_pipeline_layout_({})
    , graphics_pipeline_({})
{
    arm::log::debug("VulkanRenderer constructor");
    clear_color_ = ::vk::ClearColorValue{clear_color.r, clear_color.g, clear_color.b, clear_color.a};

    load_shaders_();
    create_graphics_pipeline_();
}

auto VulkanRenderer::begin_frame() -> void
{
    command_context_.wait_current_frame();

    for (;;)
    {
        auto [result, image_index] = swapchain_.get().acquireNextImage(
            UINT64_MAX, command_context_.current_image_available_semaphore(), VK_NULL_HANDLE);

        using enum ::vk::Result;
        if (result == eSuccess || result == eSuboptimalKHR)
        {
            current_image_index_ = image_index;
            framebuffer_resized_ |= (result == eSuboptimalKHR);
            break;
        }
        if (result == eErrorOutOfDateKHR)
        {
            swapchain_.recreate();
            framebuffer_resized_ = false;
            continue;
        }
        throw arm::Exception("Unable to aquire swapchain image ({})", ::vk::to_string(result));
    }

    auto begin_info = ::vk::CommandBufferBeginInfo{::vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    command_context_.current_command_buffer().reset();
    command_context_.current_command_buffer().begin(begin_info);
}

auto VulkanRenderer::end_frame() -> void
{
    command_context_.current_command_buffer().end();

    auto wait_for = ::vk::PipelineStageFlags{::vk::PipelineStageFlagBits::eColorAttachmentOutput};
    auto submit_info = ::vk::SubmitInfo{};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &*command_context_.current_image_available_semaphore();
    submit_info.pWaitDstStageMask = &wait_for;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &*command_context_.current_command_buffer();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &*command_context_.current_render_finished_semaphore();

    device_.graphics_queue().submit(submit_info, *command_context_.current_fence());

    auto present_info = ::vk::PresentInfoKHR{};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &*command_context_.current_render_finished_semaphore();
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &*swapchain_.get();
    present_info.pImageIndices = &current_image_index_;

    const auto present_result = device_.present_queue().presentKHR(present_info);
    if (present_result == ::vk::Result::eErrorOutOfDateKHR)
    {
        swapchain_.recreate();
    }

    command_context_.advance_frame();
}

auto VulkanRenderer::load_shaders_() -> void
{
    // TODO this is where you left off when you went gallavanting off to write a resource/asset manager
}

auto VulkanRenderer::create_graphics_pipeline_() -> void
{
    auto descriptor_sets = std::vector<::vk::DescriptorSetLayout>();
    auto push_constants = std::vector<::vk::PushConstantRange>();

    auto pipeline_layout_create_info = ::vk::PipelineLayoutCreateInfo{};
    // TODO flags?
    pipeline_layout_create_info.flags = {};

    pipeline_layout_create_info.setLayoutCount = static_cast<std::uint32_t>(descriptor_sets.size());
    pipeline_layout_create_info.pSetLayouts = descriptor_sets.data();
    pipeline_layout_create_info.pushConstantRangeCount = static_cast<std::uint32_t>(push_constants.size());
    pipeline_layout_create_info.pPushConstantRanges = push_constants.data();

    graphics_pipeline_layout_ = device_.get().createPipelineLayout(pipeline_layout_create_info);

    auto rendering_create_info = ::vk::PipelineRenderingCreateInfoKHR{};
    auto formats = std::array<::vk::Format, 1>(swapchain_.format());
    rendering_create_info.colorAttachmentCount = static_cast<std::uint32_t>(formats.size());
    rendering_create_info.pColorAttachmentFormats = formats.data();
    rendering_create_info.depthAttachmentFormat = ::vk::Format::eUndefined;

    // TODO configure this
    // TODO make sure to update pipeline_create_info.stageCount once you know it
    auto shader_stages = create_shader_stages();

    // TODO configure this
    auto vertex_input_state_create_info = ::vk::PipelineVertexInputStateCreateInfo{};

    // TODO configure this
    auto input_assembly_state_create_info = ::vk::PipelineInputAssemblyStateCreateInfo{};

    // placeholder - not currently used
    // auto tesselation_state_create_info = ::vk::PipelineTessellationStateCreateInfo{};

    // TODO configure this
    auto viewport_state_create_info = ::vk::PipelineViewportStateCreateInfo{};

    // TODO configure this
    auto rasterization_state_create_info = ::vk::PipelineRasterizationStateCreateInfo{};

    // TODO configure this
    auto multisample_state_create_info = ::vk::PipelineMultisampleStateCreateInfo{};

    // placeholder - not currently used
    // auto depth_stencil_state_create_info = ::vk::PipelineDepthStencilStateCreateInfo{};

    // TODO configure this
    auto color_blend_state_create_info = ::vk::PipelineColorBlendStateCreateInfo{};

    // TODO configure this
    auto dynamic_state_create_info = ::vk::PipelineDynamicStateCreateInfo{};

    auto pipeline_create_info = ::vk::GraphicsPipelineCreateInfo{};
    pipeline_create_info.pNext = &rendering_create_info;
    // TODO do we need flags?
    pipeline_create_info.flags = {};

    pipeline_create_info.stageCount = static_cast<std::uint32_t>(shader_stages.size());
    pipeline_create_info.pStages = shader_stages.data();

    pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
    pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
    pipeline_create_info.pTessellationState = nullptr;
    pipeline_create_info.pViewportState = &viewport_state_create_info;
    pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
    pipeline_create_info.pMultisampleState = &multisample_state_create_info;
    pipeline_create_info.pDepthStencilState = nullptr;
    pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
    pipeline_create_info.pDynamicState = &dynamic_state_create_info;

    pipeline_create_info.layout = *graphics_pipeline_layout_;

    pipeline_create_info.renderPass = nullptr;
    pipeline_create_info.subpass = 0;

    pipeline_create_info.basePipelineHandle = nullptr;
    pipeline_create_info.basePipelineIndex = 0;

    // TODO add pipeline caching
    graphics_pipeline_ = device_.get().createGraphicsPipeline(nullptr, pipeline_create_info);
    // TODO probably need some error checking, or will vulkan debug utils and validation catch it anyway?
}

}
