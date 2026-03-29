#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "graphics/shader.h"
#include "vulkan_device.h"

namespace pong
{

struct VulkanPipelineResources
{
    ::vk::raii::PipelineLayout layout;
    ::vk::raii::Pipeline pipeline;
}; // struct VulkanPipelineResources

class VulkanPipelineFactory
{
  public:
    explicit VulkanPipelineFactory(const VulkanDevice &device);
    ~VulkanPipelineFactory() = default;

    VulkanPipelineFactory(const VulkanPipelineFactory &) = delete;
    VulkanPipelineFactory &operator=(VulkanPipelineFactory &&) = delete;
    VulkanPipelineFactory(VulkanPipelineFactory &&) = delete;
    VulkanPipelineFactory &operator=(const VulkanPipelineFactory &&) = delete;

    auto create_graphics_pipeline(
        const Shader &vertex_shader,
        const Shader &fragment_shader,
        ::vk::Format swapchain_format) -> VulkanPipelineResources;

  private:
    const VulkanDevice &device_;
}; // class VulkanPipelineFactory

} // namespace pong
