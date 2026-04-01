#pragma once

#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "gpu_buffer.h"
#include "graphics/shader.h"
#include "vulkan_descriptor_pool.h"
#include "vulkan_device.h"

namespace pong
{

struct VulkanPipelineResources
{
    ::vk::raii::PipelineLayout layout;
    ::vk::raii::Pipeline pipeline;
    std::vector<::vk::raii::DescriptorSetLayout> descriptor_set_layouts;
}; // struct VulkanPipelineResources

class VulkanPipelineFactory
{
  public:
    explicit VulkanPipelineFactory(const VulkanDevice &device, const VulkanDescriptorPool &descriptor_pool);
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
    const VulkanDescriptorPool &descriptor_pool_;
}; // class VulkanPipelineFactory

} // namespace pong
