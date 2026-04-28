#pragma once

#include <array>
#include <unordered_map>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_pipeline_types.h"

namespace pong
{

class ResourceManager;
class Shader;
class VulkanDescriptorPool;
class VulkanDevice;

class VulkanPipelineManager
{
  public:
    VulkanPipelineManager(
        const VulkanDevice &device,
        VulkanDescriptorPool &descriptor_pool,
        ResourceManager &resource_manager);

    auto get_pipeline(PipelineKey key) const -> const PipelineEntry &;
    auto get_or_create_pipeline(PipelineKey key) -> const PipelineEntry &;

    auto get_per_frame_descriptor_set_layout() const -> const ::vk::raii::DescriptorSetLayout &;
    auto get_pipeline_layout() const -> const ::vk::raii::PipelineLayout &;
    auto get_default_pipeline_key() const -> PipelineKey;

    auto allocate_material_descriptor_set() -> ::vk::raii::DescriptorSet;

    auto set_color_attachment_format(::vk::Format format) -> void;

  private:
    const VulkanDevice &device_;
    ResourceManager &resource_manager_;
    VulkanDescriptorPool &descriptor_pool_;
    ::vk::raii::DescriptorSetLayout per_frame_set_0_layout_;    // set 0 = view/proj ubo
    ::vk::raii::DescriptorSetLayout per_material_set_1_layout_; // set 1 = samplers, model ubo
    std::array<::vk::PushConstantRange, 1> push_constant_ranges_;
    ::vk::raii::PipelineLayout pipeline_layout_;
    std::array<::vk::Format, 1> color_attachment_formats_;
    ::vk::Format depth_format_;
    ::vk::Format stencil_format_;
    std::unordered_map<PipelineKey, PipelineEntry> pipeline_entries_;

  private:
    auto create_per_frame_descriptor_set_layout_() -> ::vk::raii::DescriptorSetLayout;
    auto create_per_material_descriptor_set_layout_() -> ::vk::raii::DescriptorSetLayout;
    auto create_push_constant_ranges_() -> std::array<::vk::PushConstantRange, 1>;
    auto create_layout_() -> ::vk::raii::PipelineLayout;
    auto create_rendering_info_(
        std::array<::vk::Format, 1> color_attachment_formats,
        ::vk::Format depth_format,
        ::vk::Format stencil_format) -> ::vk::PipelineRenderingCreateInfo;
    auto create_pipeline_(PipelineKey key, ShaderHandle vertex_shader_handle, ShaderHandle fragment_shader_handle)
        -> PipelineEntry;
};

}
