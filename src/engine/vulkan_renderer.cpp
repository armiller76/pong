#include "vulkan_renderer.h"

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
{
    arm::log::debug("VulkanRenderer constructor");

    // create graphics pipeline
}

auto VulkanRenderer::begin_frame() -> void
{
    command_context_.wait_current_frame();
    auto [result, image_index] = swapchain_.get().acquireNextImage(
        UINT64_MAX, command_context_.current_image_available_semaphore(), VK_NULL_HANDLE);
    switch (result)
    {
        using enum ::vk::Result;
        case eSuccess:
        {
            current_image_index_ = image_index;
        }
        break;
        case eSuboptimalKHR:
        {
            current_image_index_ = image_index;
            framebuffer_resized_ = true;
        }
        break;
        case eErrorOutOfDateKHR:
        {
            swapchain_.recreate();
            // i'm supposed to retry acquire here - how?
        }
        break;
        default:
        {
            throw arm::Exception("Unknown renderer error");
        }
    }
    command_context_.current_command_buffer().reset();

    auto begin_info = ::vk::CommandBufferBeginInfo{};
    // what needs to go in the BeginInfo?
    command_context_.current_command_buffer().begin(begin_info);
}

auto VulkanRenderer::end_frame() -> void
{
    // submit command buffer, present image
}

}
