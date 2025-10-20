#include "vulkan_renderer.h"

#include "engine/vulkan_device.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

VulkanRenderer::VulkanRenderer(const VulkanDevice &device, const VulkanSurface &surface)
    : device_{device}
    , surface_{surface}
    , swapchain_{device_, surface_}
    , command_context_{device_, 2}
    , graphics_pipeline_layout_({})
    , graphics_pipeline_({})
{
    arm::log::debug("VulkanRenderer constructor");

    // create graphics pipeline
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
}
