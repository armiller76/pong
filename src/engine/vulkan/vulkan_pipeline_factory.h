#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "engine/resource_manager.h"
#include "graphics/shader.h"
#include "vulkan_descriptor_pool.h"
#include "vulkan_device.h"
#include "vulkan_gpu_buffer.h"

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
    explicit VulkanPipelineFactory(
        const VulkanDevice &device,
        const VulkanDescriptorPool &descriptor_pool,
        ResourceManager &resource_manager);
    ~VulkanPipelineFactory() = default;

    VulkanPipelineFactory(const VulkanPipelineFactory &) = delete;
    VulkanPipelineFactory &operator=(const VulkanPipelineFactory &) = delete;
    VulkanPipelineFactory(VulkanPipelineFactory &&) = delete;
    VulkanPipelineFactory &operator=(VulkanPipelineFactory &&) = delete;

    auto create_graphics_pipeline(::vk::Format swapchain_format, ::vk::Format depth_format) -> VulkanPipelineResources;

  private:
    const VulkanDevice &device_;
    const VulkanDescriptorPool &descriptor_pool_;
    ResourceManager
        &resource_manager_; // make this const again when you aren't temp loading shaders in the pipeline factory!!!
}; // class VulkanPipelineFactory

} // namespace pong
