#include "vulkan_pipeline_factory.h"

#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "gpu_buffer.h"
#include "graphics/shader.h"
#include "graphics/vertex.h"
#include "utils/log.h"
#include "vulkan_descriptor_pool.h"
#include "vulkan_device.h"

namespace pong
{

VulkanPipelineFactory::VulkanPipelineFactory(const VulkanDevice &device, const VulkanDescriptorPool &descriptor_pool)
    : device_{device}
    , descriptor_pool_{descriptor_pool}
{
    arm::log::debug("VulkanPipelineFactory constructor");
}

auto VulkanPipelineFactory::create_graphics_pipeline(
    const Shader &vertex_shader,
    const Shader &fragment_shader,
    ::vk::Format swapchain_format) -> VulkanPipelineResources
{
    // TODO bloated method, tidy up somehow
    auto vertex_ubo_layout_binding = ::vk::DescriptorSetLayoutBinding{};
    vertex_ubo_layout_binding.binding = 0;
    vertex_ubo_layout_binding.descriptorType = ::vk::DescriptorType::eUniformBuffer;
    vertex_ubo_layout_binding.descriptorCount = 1;
    vertex_ubo_layout_binding.stageFlags = ::vk::ShaderStageFlagBits::eVertex;
    vertex_ubo_layout_binding.pImmutableSamplers = nullptr;

    auto layout_bindings = std::vector{vertex_ubo_layout_binding};

    auto vertex_ubo_layout_create_info = ::vk::DescriptorSetLayoutCreateInfo{};
    vertex_ubo_layout_create_info.sType = ::vk::StructureType::eDescriptorSetLayoutCreateInfo;
    vertex_ubo_layout_create_info.pNext = nullptr;
    vertex_ubo_layout_create_info.flags = {};
    vertex_ubo_layout_create_info.bindingCount = static_cast<std::uint32_t>(layout_bindings.size());
    vertex_ubo_layout_create_info.pBindings = layout_bindings.data();

    auto descriptor_set_layouts = std::vector<::vk::raii::DescriptorSetLayout>{};
    descriptor_set_layouts.emplace_back(device_.get(), vertex_ubo_layout_create_info);

    // TODO get rid of this garbage
    auto descriptor_set_pipeline_layout_array = std::vector{*descriptor_set_layouts.at(0)};

    auto pipeline_layout_create_info = ::vk::PipelineLayoutCreateInfo{};
    pipeline_layout_create_info.setLayoutCount =
        static_cast<std::uint32_t>(descriptor_set_pipeline_layout_array.size());
    pipeline_layout_create_info.pSetLayouts = descriptor_set_pipeline_layout_array.data();
    pipeline_layout_create_info.pushConstantRangeCount = 0;
    pipeline_layout_create_info.pPushConstantRanges = nullptr;
    auto pipeline_layout = device_.get().createPipelineLayout(pipeline_layout_create_info);

    auto formats = std::array<::vk::Format, 1>{swapchain_format};
    auto rendering_create_info = ::vk::PipelineRenderingCreateInfoKHR{};
    rendering_create_info.colorAttachmentCount = static_cast<std::uint32_t>(formats.size());
    rendering_create_info.pColorAttachmentFormats = formats.data();
    rendering_create_info.depthAttachmentFormat = ::vk::Format::eUndefined;

    auto shader_stages = std::vector<::vk::PipelineShaderStageCreateInfo>();

    auto vertex_create_info = ::vk::ShaderModuleCreateInfo{};
    vertex_create_info.sType = ::vk::StructureType::eShaderModuleCreateInfo;
    vertex_create_info.codeSize = vertex_shader.spirv_view().size_bytes();
    vertex_create_info.pCode = vertex_shader.spirv_view().data();
    auto vertex = device_.get().createShaderModule(vertex_create_info);
    auto vertex_stage_create_info = ::vk::PipelineShaderStageCreateInfo{};
    vertex_stage_create_info.sType = ::vk::StructureType::ePipelineShaderStageCreateInfo;
    vertex_stage_create_info.pNext = nullptr;
    vertex_stage_create_info.flags = {};
    vertex_stage_create_info.stage = ::vk::ShaderStageFlagBits::eVertex;
    vertex_stage_create_info.module = *vertex;
    vertex_stage_create_info.pName = "main";
    vertex_stage_create_info.pSpecializationInfo = nullptr;
    shader_stages.push_back(vertex_stage_create_info);

    auto fragment_create_info = ::vk::ShaderModuleCreateInfo{};
    fragment_create_info.sType = ::vk::StructureType::eShaderModuleCreateInfo;
    fragment_create_info.codeSize = fragment_shader.spirv_view().size_bytes();
    fragment_create_info.pCode = fragment_shader.spirv_view().data();
    auto fragment = device_.get().createShaderModule(fragment_create_info);
    auto fragment_stage_create_info = ::vk::PipelineShaderStageCreateInfo{};
    fragment_stage_create_info.sType = ::vk::StructureType::ePipelineShaderStageCreateInfo;
    fragment_stage_create_info.pNext = nullptr;
    fragment_stage_create_info.flags = {};
    fragment_stage_create_info.stage = ::vk::ShaderStageFlagBits::eFragment;
    fragment_stage_create_info.module = *fragment;
    fragment_stage_create_info.pName = "main";
    fragment_stage_create_info.pSpecializationInfo = nullptr;
    shader_stages.push_back(fragment_stage_create_info);

    // TODO check configuration
    auto vertex_input_binding_description = Vertex::get_binding_description();
    auto vertex_input_attribute_descriptions = Vertex::get_attribute_descriptions();
    auto vertex_input_state_create_info = ::vk::PipelineVertexInputStateCreateInfo{};
    vertex_input_state_create_info.sType = ::vk::StructureType::ePipelineVertexInputStateCreateInfo;
    vertex_input_state_create_info.pNext = nullptr; // or ptr to VkPipelineVertexInputDivisorStateCreateInfo
    vertex_input_state_create_info.flags = {};
    vertex_input_state_create_info.vertexBindingDescriptionCount = 1; // TODO magic(ish) number
    vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_input_binding_description;
    vertex_input_state_create_info.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(vertex_input_attribute_descriptions.size());
    vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_input_attribute_descriptions.data();

    // TODO check configuration
    auto input_assembly_state_create_info = ::vk::PipelineInputAssemblyStateCreateInfo{};
    input_assembly_state_create_info.sType = ::vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
    input_assembly_state_create_info.pNext = nullptr;
    input_assembly_state_create_info.flags = {};
    input_assembly_state_create_info.topology = ::vk::PrimitiveTopology::eTriangleList;
    input_assembly_state_create_info.primitiveRestartEnable =
        VK_FALSE; // unless primitiveTopologyListRestart feature is enabled

    [[maybe_unused]] auto tesselation_state_create_info = ::vk::PipelineTessellationStateCreateInfo{};

    // TODO confirm counts should be 1 each for dynamic?
    //  Viewport and Scissor are dynamic
    //  pNext can be any combination of the following, but no more than one of each:
    //  VkPipelineViewportCoarseSampleOrderStateCreateInfoNV,
    //  VkPipelineViewportDepthClampControlCreateInfoEXT,
    //  VkPipelineViewportDepthClipControlCreateInfoEXT,
    //  VkPipelineViewportExclusiveScissorStateCreateInfoNV,
    //  VkPipelineViewportShadingRateImageStateCreateInfoNV,
    //  VkPipelineViewportSwizzleStateCreateInfoNV,
    //  VkPipelineViewportWScalingStateCreateInfoNV
    auto viewport_state_create_info = ::vk::PipelineViewportStateCreateInfo{};
    viewport_state_create_info.sType = ::vk::StructureType::ePipelineViewportStateCreateInfo;
    viewport_state_create_info.pNext = nullptr;
    viewport_state_create_info.flags = {};
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports = nullptr;
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.pScissors = nullptr;

    auto rasterization_state_create_info = ::vk::PipelineRasterizationStateCreateInfo{};
    rasterization_state_create_info.sType = ::vk::StructureType::ePipelineRasterizationStateCreateInfo;
    rasterization_state_create_info.pNext = nullptr; //    can be VkPipelineRasterizationStateRasterizationOrderAMD
    rasterization_state_create_info.flags = {};
    rasterization_state_create_info.depthClampEnable = VK_FALSE;
    rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_create_info.polygonMode = ::vk::PolygonMode::eFill;
    rasterization_state_create_info.cullMode = ::vk::CullModeFlagBits::eBack;
    rasterization_state_create_info.frontFace = ::vk::FrontFace::eCounterClockwise;
    rasterization_state_create_info.depthBiasEnable = VK_FALSE;
    rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
    rasterization_state_create_info.depthBiasClamp = 0.0f;
    rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;
    rasterization_state_create_info.lineWidth = 1.0f;

    // pNext can be any of the following, but no more than one of each:
    // VkPipelineCoverageModulationStateCreateInfoNV,
    // VkPipelineCoverageReductionStateCreateInfoNV,
    // VkPipelineCoverageToColorStateCreateInfoNV,
    // VkPipelineSampleLocationsStateCreateInfoEXT
    auto multisample_state_create_info = ::vk::PipelineMultisampleStateCreateInfo{};
    multisample_state_create_info.sType = ::vk::StructureType::ePipelineMultisampleStateCreateInfo;
    multisample_state_create_info.pNext = nullptr;
    multisample_state_create_info.flags = {};
    multisample_state_create_info.rasterizationSamples = ::vk::SampleCountFlagBits::e1;
    multisample_state_create_info.sampleShadingEnable = VK_FALSE;
    multisample_state_create_info.minSampleShading = 0.0f;
    multisample_state_create_info.pSampleMask = nullptr;
    multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
    multisample_state_create_info.alphaToOneEnable = VK_FALSE;

    [[maybe_unused]] auto depth_stencil_state_create_info = ::vk::PipelineDepthStencilStateCreateInfo{};
    auto color_blend_attachment_state = ::vk::PipelineColorBlendAttachmentState{};
    color_blend_attachment_state.blendEnable = VK_TRUE;
    color_blend_attachment_state.srcColorBlendFactor = ::vk::BlendFactor::eSrcAlpha;
    color_blend_attachment_state.dstColorBlendFactor = ::vk::BlendFactor::eOneMinusSrcAlpha;
    color_blend_attachment_state.colorBlendOp = ::vk::BlendOp::eAdd;
    color_blend_attachment_state.srcAlphaBlendFactor = ::vk::BlendFactor::eOne;
    color_blend_attachment_state.dstAlphaBlendFactor = ::vk::BlendFactor::eZero;
    color_blend_attachment_state.alphaBlendOp = ::vk::BlendOp::eAdd;
    using enum ::vk::ColorComponentFlagBits;
    color_blend_attachment_state.colorWriteMask = eR | eG | eB | eA;

    auto color_blend_state_create_info = ::vk::PipelineColorBlendStateCreateInfo{};
    color_blend_state_create_info.sType = ::vk::StructureType::ePipelineColorBlendStateCreateInfo;
    color_blend_state_create_info.pNext = nullptr;
    color_blend_state_create_info.flags = {};
    color_blend_state_create_info.logicOpEnable = VK_FALSE;
    color_blend_state_create_info.logicOp = ::vk::LogicOp::eCopy;
    color_blend_state_create_info.attachmentCount = rendering_create_info.colorAttachmentCount;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state;
    color_blend_state_create_info.blendConstants = {};

    // TODO any other dynamic states?
    auto dynamic_states = std::vector<::vk::DynamicState>{
        ::vk::DynamicState::eViewport,
        ::vk::DynamicState::eScissor,
    };
    auto dynamic_state_create_info = ::vk::PipelineDynamicStateCreateInfo{};
    dynamic_state_create_info.sType = ::vk::StructureType::ePipelineDynamicStateCreateInfo;
    dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state_create_info.pDynamicStates = dynamic_states.data();

    auto pipeline_create_info = ::vk::GraphicsPipelineCreateInfo{};
    pipeline_create_info.sType = ::vk::StructureType::eGraphicsPipelineCreateInfo;
    pipeline_create_info.pNext = &rendering_create_info;
    // TODO do we need flags?
    pipeline_create_info.flags = {};
    pipeline_create_info.stageCount = static_cast<std::uint32_t>(shader_stages.size());
    pipeline_create_info.pStages = shader_stages.data();
    pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
    pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
    pipeline_create_info.pTessellationState = nullptr; // tesselation_state_create_info
    pipeline_create_info.pViewportState = &viewport_state_create_info;
    pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
    pipeline_create_info.pMultisampleState = &multisample_state_create_info;
    pipeline_create_info.pDepthStencilState = nullptr; // depth_stencil_state_create_info
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

    return {
        .layout = std::move(pipeline_layout),
        .pipeline = std::move(pipeline),
        .descriptor_set_layouts = std::move(descriptor_set_layouts),
    };
}

} // namespace pong
