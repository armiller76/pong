#include "vulkan_renderer.h"

namespace pong
{

auto VulkanRenderer::begin_frame() -> void
{
    // acquire next swapchain image, wait on fence
}

auto VulkanRenderer::begin_render_pass() -> void
{
    // start render pass, bind pipeline
}

auto VulkanRenderer::draw() -> void
{
    // record draw commands
    // (or, accept a drawable interface)
}

auto VulkanRenderer::end_render_pass() -> void
{
    // finish render pass
}

auto VulkanRenderer::end_frame() -> void
{
    // submit command buffer, present image
}

}
