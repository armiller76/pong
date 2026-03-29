#include "vulkan_pipeline_factory.h"

#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "graphics/shader.h"
#include "graphics/vertex.h"
#include "vulkan_device.h"

namespace pong
{

VulkanPipelineFactory::VulkanPipelineFactory(const VulkanDevice &device)
    : device_{device}
{
}

auto VulkanPipelineFactory::create_graphics_pipeline(
    const Shader &vertex_shader,
    const Shader &fragment_shader,
    ::vk::Format swapchain_format) -> VulkanPipelineResources
{
    // TODO descriptor sets and push constants empty for now until we start using uniforms and such
    auto descriptor_sets = std::vector<::vk::DescriptorSetLayout>();
    auto push_constants = std::vector<::vk::PushConstantRange>();
    auto pipeline_layout_create_info = ::vk::PipelineLayoutCreateInfo{};
    pipeline_layout_create_info.setLayoutCount = static_cast<std::uint32_t>(descriptor_sets.size());
    pipeline_layout_create_info.pSetLayouts = descriptor_sets.data();
    pipeline_layout_create_info.pushConstantRangeCount = static_cast<std::uint32_t>(push_constants.size());
    pipeline_layout_create_info.pPushConstantRanges = push_constants.data();
    auto pipeline_layout = device_.get().createPipelineLayout(pipeline_layout_create_info);

    auto formats = std::array<::vk::Format, 1>(swapchain_format);
    auto rendering_create_info = ::vk::PipelineRenderingCreateInfoKHR{};
    rendering_create_info.colorAttachmentCount = static_cast<std::uint32_t>(formats.size());
    rendering_create_info.pColorAttachmentFormats = formats.data();
    rendering_create_info.depthAttachmentFormat = ::vk::Format::eUndefined;

    auto shader_stages = std::vector<::vk::raii::ShaderModule>();
    shader_stages.reserve(2zu);

    auto vertex_create_info = ::vk::ShaderModuleCreateInfo{};
    vertex_create_info.codeSize = vertex_shader.spirv_view().size_bytes();
    vertex_create_info.pCode = vertex_shader.spirv_view().data();
    shader_stages[0] = device_.get().createShaderModule(vertex_create_info);

    auto fragment_create_info = ::vk::ShaderModuleCreateInfo{};
    fragment_create_info.codeSize = fragment_shader.spirv_view().size_bytes();
    fragment_create_info.pCode = fragment_shader.spirv_view().data();
    shader_stages[1] = device_.get().createShaderModule(fragment_create_info);

    // TODO check configuration
    auto vertex_input_binding_description = Vertex::get_binding_description();
    auto vertex_input_attribute_descriptions = Vertex::get_attribute_descriptions();
    auto vertex_input_state_create_info = ::vk::PipelineVertexInputStateCreateInfo{};
    vertex_input_state_create_info.sType = ::vk::StructureType::ePipelineVertexInputStateCreateInfo;
    vertex_input_state_create_info.pNext = nullptr;                   // TODO need pNext?
    vertex_input_state_create_info.flags = {};                        // TODO need flags?
    vertex_input_state_create_info.vertexBindingDescriptionCount = 1; // TODO magic number
    vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_input_binding_description;
    vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_input_attribute_descriptions.size();
    vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_input_attribute_descriptions.data();

    // TODO check configuration
    auto input_assembly_state_create_info = ::vk::PipelineInputAssemblyStateCreateInfo{};
    input_assembly_state_create_info.sType = ::vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
    input_assembly_state_create_info.pNext = nullptr; // TODO need pNext?
    input_assembly_state_create_info.flags = {};      // TODO need flags?
    input_assembly_state_create_info.topology = ::vk::PrimitiveTopology::eTriangleList;
    input_assembly_state_create_info.primitiveRestartEnable = false; // TODO true?

    // placeholder - not currently used
    // auto tesselation_state_create_info = ::vk::PipelineTessellationStateCreateInfo{};

    // TODO configure this
    // Viewport and Scissor: We usually set these to 'dynamic' so we don't
    // have to recreate the whole pipeline when the window is resized.
    auto viewport_state_create_info = ::vk::PipelineViewportStateCreateInfo{};
    viewport_state_create_info.sType;         //     VkStructureType
    viewport_state_create_info.pNext;         //     const void*
    viewport_state_create_info.flags;         //     VkPipelineViewportStateCreateFlags
    viewport_state_create_info.viewportCount; //     uint32_t
    viewport_state_create_info.pViewports;    //     const VkViewport*
    viewport_state_create_info.scissorCount;  //     uint32_t
    viewport_state_create_info.pScissors;     //     const VkRect2D*
    viewport_state_create_info;               // } VkPipelineViewportStateCreateInfo;

    // TODO configure this
    // Standard rasterizer: fill the triangles, cull back faces,
    // and decide if clockwise or counter-clockwise is "front".
    auto rasterization_state_create_info = ::vk::PipelineRasterizationStateCreateInfo{};

    // TODO configure this
    // Set this to 1 sample (no MSAA) for now.
    auto multisample_state_create_info = ::vk::PipelineMultisampleStateCreateInfo{};

    // placeholder - not currently used
    // auto depth_stencil_state_create_info = ::vk::PipelineDepthStencilStateCreateInfo{};

    // TODO configure this
    // For now, no transparency (blending disabled).
    auto color_blend_state_create_info = ::vk::PipelineColorBlendStateCreateInfo{};

    // TODO configure this
    // Tell Vulkan which parts of the pipeline state (like Viewport)
    // we intend to change during the draw call.
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
    pipeline_create_info.pViewportState = &pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
    pipeline_create_info.pMultisampleState = &multisample_state_create_info;
    pipeline_create_info.pDepthStencilState = nullptr;
    pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
    pipeline_create_info.pDynamicState = &dynamic_state_create_info;

    pipeline_create_info.layout = *pipeline_layout;

    pipeline_create_info.renderPass = nullptr;
    pipeline_create_info.subpass = 0;

    pipeline_create_info.basePipelineHandle = nullptr;
    pipeline_create_info.basePipelineIndex = 0;

    // TODO add pipeline caching
    auto pipeline = device_.get().createGraphicsPipeline(nullptr, pipeline_create_info);
    // TODO probably need some error checking, or will vulkan debug utils and validation catch it anyway?

    return {.layout = std::move(pipeline_layout), .pipeline = std::move(pipeline)};
};

} // namespace pong
