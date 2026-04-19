#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace pong
{

class ResourceLoader;
class ResourceManager;
class VulkanDevice;
class VulkanDescriptorPool;
class VulkanGpuBuffer;

struct VulkanPipelineResources
{
    ::vk::raii::PipelineLayout layout;
    ::vk::raii::Pipeline pipeline;
    ::vk::raii::DescriptorSetLayout descriptor_set_layout;
}; // struct VulkanPipelineResources

class VulkanPipelineFactory
{
  public:
    explicit VulkanPipelineFactory(const VulkanDevice &device, ResourceManager &resource_manager);
    ~VulkanPipelineFactory() = default;

    VulkanPipelineFactory(const VulkanPipelineFactory &) = delete;
    VulkanPipelineFactory &operator=(const VulkanPipelineFactory &) = delete;
    VulkanPipelineFactory(VulkanPipelineFactory &&) = delete;
    VulkanPipelineFactory &operator=(VulkanPipelineFactory &&) = delete;

    auto create_graphics_pipeline(::vk::Format swapchain_format, ::vk::Format depth_format) -> VulkanPipelineResources;

  private:
    const VulkanDevice &device_;
    ResourceManager
        &resource_manager_; // make this const again when you aren't temp loading shaders in the pipeline factory!!!
}; // class VulkanPipelineFactory

} // namespace pong
