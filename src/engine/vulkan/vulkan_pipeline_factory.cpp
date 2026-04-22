#include "vulkan_pipeline_factory.h"

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "core/resource_handles.h"
#include "engine/resource_manager.h"
#include "graphics/shader.h"
#include "graphics/vertex.h"
#include "utils/hash.h"
#include "utils/log.h"
#include "vulkan_device.h"
#include "vulkan_gpu_buffer.h"
#include "vulkan_render_utils.h"

namespace pong
{

static auto vertex_input_binding_description_factory() -> ::vk::VertexInputBindingDescription;
static auto vertex_input_attribute_descriptions_factory() -> std::vector<::vk::VertexInputAttributeDescription>;

VulkanPipelineFactory::VulkanPipelineFactory(const VulkanDevice &device, ResourceManager &resource_manager)
    : device_{device}
    , resource_manager_{resource_manager}
{
    arm::log::debug("VulkanPipelineFactory constructor");
}

auto VulkanPipelineFactory::create_graphics_pipeline(::vk::Format swapchain_format, ::vk::Format depth_format)
    -> VulkanPipelineResources
{
    // ---- SET 0 ---- //
    auto view_proj_ubo_layout_binding = ::vk::DescriptorSetLayoutBinding{};
    view_proj_ubo_layout_binding.binding = 0u;
    view_proj_ubo_layout_binding.descriptorType = ::vk::DescriptorType::eUniformBuffer;
    view_proj_ubo_layout_binding.descriptorCount = 1u;
    view_proj_ubo_layout_binding.stageFlags = ::vk::ShaderStageFlagBits::eVertex;
    view_proj_ubo_layout_binding.pImmutableSamplers = nullptr;

    // ---- SET 0 ---- //
    auto per_frame_layout_bindings = std::vector{
        view_proj_ubo_layout_binding,
    };

    // ---- SET 0 ---- //
    auto per_frame_descriptor_set_layout_create_info = ::vk::DescriptorSetLayoutCreateInfo{};
    per_frame_descriptor_set_layout_create_info.sType = ::vk::StructureType::eDescriptorSetLayoutCreateInfo;
    per_frame_descriptor_set_layout_create_info.pNext = nullptr;
    per_frame_descriptor_set_layout_create_info.flags = {};
    per_frame_descriptor_set_layout_create_info.bindingCount =
        static_cast<std::uint32_t>(per_frame_layout_bindings.size());
    per_frame_descriptor_set_layout_create_info.pBindings = per_frame_layout_bindings.data();

    // ---- SET 0 ---- //
    auto per_frame_descriptor_set_layout =
        ::vk::raii::DescriptorSetLayout{device_.native_handle(), per_frame_descriptor_set_layout_create_info};

    // ---- SET 1 ---- //
    auto base_sampler_layout_binding = ::vk::DescriptorSetLayoutBinding{};
    base_sampler_layout_binding.binding = 0u;
    base_sampler_layout_binding.descriptorType = ::vk::DescriptorType::eCombinedImageSampler;
    base_sampler_layout_binding.descriptorCount = 1u;
    base_sampler_layout_binding.stageFlags = ::vk::ShaderStageFlagBits::eFragment;
    base_sampler_layout_binding.pImmutableSamplers = nullptr;

    // ---- SET 1 ---- //
    auto metal_sampler_layout_binding = ::vk::DescriptorSetLayoutBinding{};
    metal_sampler_layout_binding.binding = 1u;
    metal_sampler_layout_binding.descriptorType = ::vk::DescriptorType::eCombinedImageSampler;
    metal_sampler_layout_binding.descriptorCount = 1u;
    metal_sampler_layout_binding.stageFlags = ::vk::ShaderStageFlagBits::eFragment;
    metal_sampler_layout_binding.pImmutableSamplers = nullptr;

    // ---- SET 1 ---- //
    auto normal_sampler_layout_binding = ::vk::DescriptorSetLayoutBinding{};
    normal_sampler_layout_binding.binding = 2u;
    normal_sampler_layout_binding.descriptorType = ::vk::DescriptorType::eCombinedImageSampler;
    normal_sampler_layout_binding.descriptorCount = 1u;
    normal_sampler_layout_binding.stageFlags = ::vk::ShaderStageFlagBits::eFragment;
    normal_sampler_layout_binding.pImmutableSamplers = nullptr;

    // ---- SET 1 ---- //
    auto material_ubo_layout_binding = ::vk::DescriptorSetLayoutBinding{};
    material_ubo_layout_binding.binding = 3u;
    material_ubo_layout_binding.descriptorType = ::vk::DescriptorType::eUniformBuffer;
    material_ubo_layout_binding.descriptorCount = 1u;
    material_ubo_layout_binding.stageFlags = ::vk::ShaderStageFlagBits::eFragment;
    material_ubo_layout_binding.pImmutableSamplers = nullptr;

    // ---- SET 1 ---- //
    auto per_material_layout_bindings = std::vector{
        base_sampler_layout_binding,
        metal_sampler_layout_binding,
        normal_sampler_layout_binding,
        material_ubo_layout_binding,
    };

    // ---- SET 1 ---- //
    auto per_material_descriptor_set_layout_create_info = ::vk::DescriptorSetLayoutCreateInfo{};
    per_material_descriptor_set_layout_create_info.sType = ::vk::StructureType::eDescriptorSetLayoutCreateInfo;
    per_material_descriptor_set_layout_create_info.pNext = nullptr;
    per_material_descriptor_set_layout_create_info.flags = {};
    per_material_descriptor_set_layout_create_info.bindingCount =
        static_cast<std::uint32_t>(per_material_layout_bindings.size());
    per_material_descriptor_set_layout_create_info.pBindings = per_material_layout_bindings.data();

    // ---- SET 1 ---- //
    auto per_material_descriptor_set_layout =
        ::vk::raii::DescriptorSetLayout{device_.native_handle(), per_material_descriptor_set_layout_create_info};

    auto pipeline_descriptor_set_layouts = std::vector{
        *per_frame_descriptor_set_layout,
        *per_material_descriptor_set_layout,
    };

    auto model_push_constant_range = ::vk::PushConstantRange{};
    model_push_constant_range.stageFlags = ::vk::ShaderStageFlagBits::eVertex;
    model_push_constant_range.offset = 0;
    model_push_constant_range.size = sizeof(::glm::mat4);

    auto pipeline_layout_create_info = ::vk::PipelineLayoutCreateInfo{};
    pipeline_layout_create_info.setLayoutCount = static_cast<std::uint32_t>(pipeline_descriptor_set_layouts.size());
    pipeline_layout_create_info.pSetLayouts = pipeline_descriptor_set_layouts.data();
    pipeline_layout_create_info.pushConstantRangeCount = 1;
    pipeline_layout_create_info.pPushConstantRanges = &model_push_constant_range;
    auto pipeline_layout = device_.native_handle().createPipelineLayout(pipeline_layout_create_info);

    auto formats = std::array{swapchain_format};
    auto rendering_create_info = ::vk::PipelineRenderingCreateInfoKHR{};
    rendering_create_info.colorAttachmentCount = static_cast<std::uint32_t>(formats.size());
    rendering_create_info.pColorAttachmentFormats = formats.data();
    rendering_create_info.depthAttachmentFormat = depth_format;

    // TODO hardcoded shader
    auto &vertex_shader = resource_manager_.get<Shader>(ShaderHandle{hash_string("simple.vert")});
    auto vertex_create_info = ::vk::ShaderModuleCreateInfo{};
    vertex_create_info.sType = ::vk::StructureType::eShaderModuleCreateInfo;
    vertex_create_info.codeSize = vertex_shader.spirv_view().size_bytes();
    vertex_create_info.pCode = vertex_shader.spirv_view().data();
    auto vertex = device_.native_handle().createShaderModule(vertex_create_info);
    auto vertex_stage_create_info = ::vk::PipelineShaderStageCreateInfo{};
    vertex_stage_create_info.sType = ::vk::StructureType::ePipelineShaderStageCreateInfo;
    vertex_stage_create_info.pNext = nullptr;
    vertex_stage_create_info.flags = {};
    vertex_stage_create_info.stage = ::vk::ShaderStageFlagBits::eVertex;
    vertex_stage_create_info.module = *vertex;
    vertex_stage_create_info.pName = "main";
    vertex_stage_create_info.pSpecializationInfo = nullptr;

    // TODO hardcoded shader
    auto &fragment_shader = resource_manager_.get<Shader>(ShaderHandle{hash_string("simple.frag")});
    auto fragment_create_info = ::vk::ShaderModuleCreateInfo{};
    fragment_create_info.sType = ::vk::StructureType::eShaderModuleCreateInfo;
    fragment_create_info.codeSize = fragment_shader.spirv_view().size_bytes();
    fragment_create_info.pCode = fragment_shader.spirv_view().data();
    auto fragment = device_.native_handle().createShaderModule(fragment_create_info);
    auto fragment_stage_create_info = ::vk::PipelineShaderStageCreateInfo{};
    fragment_stage_create_info.sType = ::vk::StructureType::ePipelineShaderStageCreateInfo;
    fragment_stage_create_info.pNext = nullptr;
    fragment_stage_create_info.flags = {};
    fragment_stage_create_info.stage = ::vk::ShaderStageFlagBits::eFragment;
    fragment_stage_create_info.module = *fragment;
    fragment_stage_create_info.pName = "main";
    fragment_stage_create_info.pSpecializationInfo = nullptr;

    auto shader_stages = std::array{
        vertex_stage_create_info,
        fragment_stage_create_info,
    };

    // TODO check configuration
    auto vertex_input_binding_description = vertex_input_binding_description_factory();
    auto vertex_input_attribute_descriptions = vertex_input_attribute_descriptions_factory();
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

    auto depth_stencil_state_create_info = ::vk::PipelineDepthStencilStateCreateInfo{};
    depth_stencil_state_create_info.sType = ::vk::StructureType::ePipelineDepthStencilStateCreateInfo;
    depth_stencil_state_create_info.pNext = nullptr;
    depth_stencil_state_create_info.flags = {};
    depth_stencil_state_create_info.depthTestEnable = ::vk::True;
    depth_stencil_state_create_info.depthWriteEnable = ::vk::True;
    depth_stencil_state_create_info.depthCompareOp = ::vk::CompareOp::eLess;
    // future use:
    //    depth_stencil_state_create_info.depthBoundsTestEnable = ;
    //    depth_stencil_state_create_info.stencilTestEnable = ;
    //    depth_stencil_state_create_info.front = ;
    //    depth_stencil_state_create_info.back = ;
    //    depth_stencil_state_create_info.minDepthBounds = ;

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
    pipeline_create_info.pDepthStencilState = &depth_stencil_state_create_info;
    pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
    pipeline_create_info.pDynamicState = &dynamic_state_create_info;
    pipeline_create_info.layout = *pipeline_layout;
    pipeline_create_info.renderPass = nullptr;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = nullptr;
    pipeline_create_info.basePipelineIndex = 0;

    // TODO add pipeline caching
    auto pipeline = device_.native_handle().createGraphicsPipeline(nullptr, pipeline_create_info);
    // TODO probably need some error checking, or will vulkan debug utils and validation catch it anyway?

    return {
        .layout = std::move(pipeline_layout),
        .pipeline = std::move(pipeline),
        .per_frame_descriptor_set_layout = std::move(per_frame_descriptor_set_layout),
        .per_material_descriptor_set_layout = std::move(per_material_descriptor_set_layout),
    };
}

inline auto vertex_input_binding_description_factory() -> ::vk::VertexInputBindingDescription
{
    auto result = ::vk::VertexInputBindingDescription{};
    result.binding = 0;
    result.stride = sizeof(Vertex);
    result.inputRate = ::vk::VertexInputRate::eVertex;
    return result;
}

inline auto vertex_input_attribute_descriptions_factory() -> std::vector<::vk::VertexInputAttributeDescription>
{
    auto position_entry = ::vk::VertexInputAttributeDescription{};
    position_entry.location = 0;
    position_entry.binding = 0;
    position_entry.format = ::vk::Format::eR32G32B32Sfloat;
    position_entry.offset = offsetof(Vertex, position);
    auto color_entry = ::vk::VertexInputAttributeDescription{};
    color_entry.location = 1;
    color_entry.binding = 0;
    color_entry.format = ::vk::Format::eR32G32B32Sfloat;
    color_entry.offset = offsetof(Vertex, color);
    auto normal_entry = ::vk::VertexInputAttributeDescription{};
    normal_entry.location = 2;
    normal_entry.binding = 0;
    normal_entry.format = ::vk::Format::eR32G32B32Sfloat;
    normal_entry.offset = offsetof(Vertex, normal);
    auto texture_coordinate_entry = ::vk::VertexInputAttributeDescription{};
    texture_coordinate_entry.location = 3;
    texture_coordinate_entry.binding = 0;
    texture_coordinate_entry.format = ::vk::Format::eR32G32Sfloat;
    texture_coordinate_entry.offset = offsetof(Vertex, uv);

    return std::vector{
        position_entry,
        color_entry,
        normal_entry,
        texture_coordinate_entry,
    };
}

} // namespace pong
