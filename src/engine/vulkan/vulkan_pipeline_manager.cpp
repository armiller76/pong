#include "engine/vulkan/vulkan_pipeline_manager.h"

#include <array>
#include <expected>
#include <utility>

#include <vulkan/vulkan_raii.hpp>

#include "core/resource_handles.h"
#include "engine/engine_error.h"
#include "engine/resource_manager.h"
#include "engine/vulkan/vulkan_descriptor_pool.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_pipeline_types.h"
#include "graphics/glm_wrapper.h" // IWYU pragma: keep
#include "graphics/shader.h"
#include "graphics/types.h"
#include "graphics/vertex.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace
{

constexpr auto get_vertex_input_binding_description() -> ::vk::VertexInputBindingDescription
{
    auto result = ::vk::VertexInputBindingDescription{};
    result.binding = 0;
    result.stride = sizeof(pong::Vertex);
    result.inputRate = ::vk::VertexInputRate::eVertex;
    return result;
}

constexpr auto get_vertex_input_attribute_descriptions() -> std::vector<::vk::VertexInputAttributeDescription>
{
    auto position_entry = ::vk::VertexInputAttributeDescription{};
    position_entry.location = 0;
    position_entry.binding = 0;
    position_entry.format = ::vk::Format::eR32G32B32Sfloat;
    position_entry.offset = offsetof(pong::Vertex, position);
    auto color_entry = ::vk::VertexInputAttributeDescription{};
    color_entry.location = 1;
    color_entry.binding = 0;
    color_entry.format = ::vk::Format::eR32G32B32Sfloat;
    color_entry.offset = offsetof(pong::Vertex, color);
    auto normal_entry = ::vk::VertexInputAttributeDescription{};
    normal_entry.location = 2;
    normal_entry.binding = 0;
    normal_entry.format = ::vk::Format::eR32G32B32Sfloat;
    normal_entry.offset = offsetof(pong::Vertex, normal);
    auto texture_coordinate_entry = ::vk::VertexInputAttributeDescription{};
    texture_coordinate_entry.location = 3;
    texture_coordinate_entry.binding = 0;
    texture_coordinate_entry.format = ::vk::Format::eR32G32Sfloat;
    texture_coordinate_entry.offset = offsetof(pong::Vertex, uv);
    auto tangent_entry = ::vk::VertexInputAttributeDescription{};
    tangent_entry.location = 4;
    tangent_entry.binding = 0;
    tangent_entry.format = ::vk::Format::eR32G32B32A32Sfloat;
    tangent_entry.offset = offsetof(pong::Vertex, tangent);

    return std::vector{
        position_entry,
        color_entry,
        normal_entry,
        texture_coordinate_entry,
        tangent_entry,
    };
}

} // anonymous namespace

namespace pong
{

VulkanPipelineManager::VulkanPipelineManager(
    const VulkanDevice &device,
    VulkanDescriptorPool &descriptor_pool,
    ResourceManager &resource_manager)
    : device_{device}
    , resource_manager_{resource_manager}
    , descriptor_pool_{descriptor_pool}
    , per_frame_set_0_layout_{create_per_frame_descriptor_set_layout_()}
    , per_material_set_1_layout_{create_per_material_descriptor_set_layout_()}
    , push_constant_ranges_{create_push_constant_ranges_()}
    , pipeline_layout_{create_layout_()}
    , depth_format_{device_.choose_depth_format()}
    , stencil_format_{::vk::Format::eUndefined} // TODO magic number
{
    arm::log::debug("VulkanPipelineManager constructor");
}

auto VulkanPipelineManager::get_pipeline(PipelineKey key) const -> const PipelineEntry &
{
    if (pipeline_entries_.contains(key))
    {
        return pipeline_entries_.at(key);
    }
    else
    {
        throw arm::Exception("pipeline does not exist");
    }
}

auto VulkanPipelineManager::get_or_create_pipeline(PipelineKey key) -> const PipelineEntry &
{
    auto [entry, inserted] = pipeline_entries_.try_emplace(
        key,
        create_pipeline_(key, resource_manager_.default_vertex_shader(), resource_manager_.default_fragment_shader()));
    if (!inserted)
    {
        return pipeline_entries_.at(key);
    }
    else
    {
        return entry->second;
    }
}

auto VulkanPipelineManager::get_per_frame_descriptor_set_layout() const -> const ::vk::raii::DescriptorSetLayout &
{
    return per_frame_set_0_layout_;
}

auto VulkanPipelineManager::get_pipeline_layout() const -> const ::vk::raii::PipelineLayout &
{
    return pipeline_layout_;
}

auto VulkanPipelineManager::get_default_pipeline_key() const -> PipelineKey
{
    return {PassType::Main, AlphaMode::Opaque, RasterState::Default, VertexInput::NOT_IMPLEMENTED, ShaderFeature{}};
}

auto VulkanPipelineManager::allocate_material_descriptor_set() -> ::vk::raii::DescriptorSet
{
    return descriptor_pool_.allocate_material_descriptor_set(per_material_set_1_layout_);
}

auto VulkanPipelineManager::set_color_attachment_format(::vk::Format format) -> void
{
    if (format == ::vk::Format::eUndefined)
    {
        throw arm::Exception("invalid format");
    }
    if (!color_attachment_formats_.empty() && color_attachment_formats_[0] != ::vk::Format::eUndefined)
    {
        arm::log::error("max color attachment formats currently 1, overwriting");
    }
    color_attachment_formats_ = std::array{
        format,
    };
}

auto VulkanPipelineManager::create_per_frame_descriptor_set_layout_() -> ::vk::raii::DescriptorSetLayout
{
    // ---- SET 0 ---- //
    auto view_proj_ubo_layout_binding = ::vk::DescriptorSetLayoutBinding{};
    view_proj_ubo_layout_binding.binding = 0u;
    view_proj_ubo_layout_binding.descriptorType = ::vk::DescriptorType::eUniformBuffer;
    view_proj_ubo_layout_binding.descriptorCount = 1u;
    view_proj_ubo_layout_binding.stageFlags = ::vk::ShaderStageFlagBits::eVertex;
    view_proj_ubo_layout_binding.pImmutableSamplers = nullptr;

    // ---- SET 0 ---- //
    auto light_ubo_layout_binding = ::vk::DescriptorSetLayoutBinding{};
    light_ubo_layout_binding.binding = 1u;
    light_ubo_layout_binding.descriptorType = ::vk::DescriptorType::eUniformBuffer;
    light_ubo_layout_binding.descriptorCount = 1u;
    light_ubo_layout_binding.stageFlags = ::vk::ShaderStageFlagBits::eFragment;
    light_ubo_layout_binding.pImmutableSamplers = nullptr;

    // ---- SET 0 ---- //
    auto per_frame_layout_bindings = std::vector{
        view_proj_ubo_layout_binding,
        light_ubo_layout_binding,
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

    auto result = check_vk_expected(
        device_.native_handle().createDescriptorSetLayout(per_frame_descriptor_set_layout_create_info));
    if (!result)
    {
        // TODO avoid exception once error handling is available
        throw arm::Exception("error: {} (\"{}\")", to_string(result.error().code), result.error().message);
    }

    return std::move(result.value());
}

auto VulkanPipelineManager::create_per_material_descriptor_set_layout_() -> ::vk::raii::DescriptorSetLayout
{
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
    auto result = check_vk_expected(
        device_.native_handle().createDescriptorSetLayout(per_material_descriptor_set_layout_create_info));
    if (!result)
    {
        // TODO avoid exception once error handling is available
        throw arm::Exception("error: {} (\"{}\")", to_string(result.error().code), result.error().message);
    }

    return std::move(result.value());
}

auto VulkanPipelineManager::create_push_constant_ranges_() -> std::array<::vk::PushConstantRange, 1>
{
    auto model_push_constant_range = ::vk::PushConstantRange{};
    model_push_constant_range.stageFlags = ::vk::ShaderStageFlagBits::eVertex;
    model_push_constant_range.offset = 0;
    model_push_constant_range.size = sizeof(::glm::mat4);

    return std::array{
        model_push_constant_range,
    };
}

auto VulkanPipelineManager::create_layout_() -> ::vk::raii::PipelineLayout
{
    auto pipeline_descriptor_set_layouts = std::vector{
        *per_frame_set_0_layout_,
        *per_material_set_1_layout_,
    };

    auto pipeline_layout_create_info = ::vk::PipelineLayoutCreateInfo{};
    pipeline_layout_create_info.setLayoutCount = static_cast<std::uint32_t>(pipeline_descriptor_set_layouts.size());
    pipeline_layout_create_info.pSetLayouts = pipeline_descriptor_set_layouts.data();
    pipeline_layout_create_info.pushConstantRangeCount = static_cast<std::uint32_t>(push_constant_ranges_.size());
    pipeline_layout_create_info.pPushConstantRanges = push_constant_ranges_.data();

    auto result = check_vk_expected(device_.native_handle().createPipelineLayout(pipeline_layout_create_info));
    if (!result)
    {
        // TODO avoid exception once error handling is available
        throw arm::Exception("error: {} (\"{}\")", to_string(result.error().code), result.error().message);
    }

    return std::move(result.value());
}

auto VulkanPipelineManager::create_pipeline_(
    PipelineKey key,
    ShaderHandle vertex_shader_handle,
    ShaderHandle fragment_shader_handle) -> PipelineEntry
{
    auto &vertex_shader = resource_manager_.get<Shader>(vertex_shader_handle);
    auto vertex_stage_create_info = ::vk::PipelineShaderStageCreateInfo{};
    vertex_stage_create_info.sType = ::vk::StructureType::ePipelineShaderStageCreateInfo;
    vertex_stage_create_info.pNext = nullptr;
    vertex_stage_create_info.flags = {};
    vertex_stage_create_info.stage = ::vk::ShaderStageFlagBits::eVertex;
    vertex_stage_create_info.module = vertex_shader.module_handle();
    vertex_stage_create_info.pName = vertex_shader.entry_point_c_str();
    vertex_stage_create_info.pSpecializationInfo = nullptr;

    auto &fragment_shader = resource_manager_.get<Shader>(fragment_shader_handle);
    auto fragment_stage_create_info = ::vk::PipelineShaderStageCreateInfo{};
    fragment_stage_create_info.sType = ::vk::StructureType::ePipelineShaderStageCreateInfo;
    fragment_stage_create_info.pNext = nullptr;
    fragment_stage_create_info.flags = {};
    fragment_stage_create_info.stage = ::vk::ShaderStageFlagBits::eFragment;
    fragment_stage_create_info.module = fragment_shader.module_handle();
    fragment_stage_create_info.pName = fragment_shader.entry_point_c_str();
    fragment_stage_create_info.pSpecializationInfo = nullptr;

    auto shader_stages = std::array{
        vertex_stage_create_info,
        fragment_stage_create_info,
    };

    auto vertex_input_binding_description = get_vertex_input_binding_description();
    auto vertex_input_attribute_descriptions = get_vertex_input_attribute_descriptions();
    auto vertex_input_state_create_info = ::vk::PipelineVertexInputStateCreateInfo{};
    vertex_input_state_create_info.sType = ::vk::StructureType::ePipelineVertexInputStateCreateInfo;
    vertex_input_state_create_info.pNext = nullptr;
    vertex_input_state_create_info.flags = {};
    vertex_input_state_create_info.vertexBindingDescriptionCount = 1; // TODO magic(ish) number
    vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_input_binding_description;
    vertex_input_state_create_info.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(vertex_input_attribute_descriptions.size());
    vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_input_attribute_descriptions.data();

    auto input_assembly_state_create_info = ::vk::PipelineInputAssemblyStateCreateInfo{};
    input_assembly_state_create_info.sType = ::vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
    input_assembly_state_create_info.pNext = nullptr;
    input_assembly_state_create_info.flags = {};
    switch (key.raster_state)
    {
        case RasterState::Default:
        case RasterState::Double:
        {
            input_assembly_state_create_info.topology = ::vk::PrimitiveTopology::eTriangleList;
        }
        break;
        case RasterState::Wireframe:
        {
            input_assembly_state_create_info.topology = ::vk::PrimitiveTopology::eLineList;
        }
        break;
        case RasterState::RasterStateCount:
        default:
        {
            throw arm::Exception("unknown raster state");
        }
    }
    input_assembly_state_create_info.primitiveRestartEnable = ::vk::False;

    auto tesselation_state_create_info = ::vk::PipelineTessellationStateCreateInfo{};
    tesselation_state_create_info.sType = ::vk::StructureType::ePipelineTessellationStateCreateInfo;
    tesselation_state_create_info.pNext = nullptr;
    tesselation_state_create_info.flags = {};
    tesselation_state_create_info.patchControlPoints = 0;

    //  Viewport and Scissor are dynamic, viewport and scissor pointers are ignored
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
    rasterization_state_create_info.pNext = nullptr;
    rasterization_state_create_info.flags = {};
    rasterization_state_create_info.depthClampEnable = ::vk::False;
    rasterization_state_create_info.rasterizerDiscardEnable = ::vk::False;
    switch (key.raster_state)
    {
        case RasterState::Default:
        {
            rasterization_state_create_info.cullMode = ::vk::CullModeFlagBits::eBack;
            rasterization_state_create_info.polygonMode = ::vk::PolygonMode::eFill;
        }
        break;
        case RasterState::Double:
        {
            rasterization_state_create_info.cullMode = ::vk::CullModeFlagBits::eNone;
            rasterization_state_create_info.polygonMode = ::vk::PolygonMode::eFill;
        }
        break;
        case RasterState::Wireframe:
        {
            rasterization_state_create_info.cullMode = ::vk::CullModeFlagBits::eNone;
            rasterization_state_create_info.polygonMode = ::vk::PolygonMode::eLine;
        }
        break;
        case RasterState::RasterStateCount:
        default:
        {
            throw arm::Exception("unknown raster state");
        }
    }
    rasterization_state_create_info.frontFace = ::vk::FrontFace::eCounterClockwise;
    rasterization_state_create_info.depthBiasEnable = ::vk::False;
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
    multisample_state_create_info.sampleShadingEnable = ::vk::False;
    multisample_state_create_info.minSampleShading = 0.0f;
    multisample_state_create_info.pSampleMask = nullptr;
    multisample_state_create_info.alphaToCoverageEnable = ::vk::False;
    multisample_state_create_info.alphaToOneEnable = ::vk::False;

    auto depth_stencil_state_create_info = ::vk::PipelineDepthStencilStateCreateInfo{};
    depth_stencil_state_create_info.sType = ::vk::StructureType::ePipelineDepthStencilStateCreateInfo;
    depth_stencil_state_create_info.pNext = nullptr;
    depth_stencil_state_create_info.flags = {};
    depth_stencil_state_create_info.depthTestEnable = ::vk::True;
    switch (key.alpha_mode)
    {
        case AlphaMode::Blend:
        {
            depth_stencil_state_create_info.depthWriteEnable = ::vk::False;
        }
        break;
        case AlphaMode::Mask:
        case AlphaMode::Opaque:
        {
            depth_stencil_state_create_info.depthWriteEnable = ::vk::True;
        }
        break;
        default:
        {
            throw arm::Exception("unknown alpha mode");
        }
    }
    depth_stencil_state_create_info.depthCompareOp = ::vk::CompareOp::eLess;
    depth_stencil_state_create_info.depthBoundsTestEnable = ::vk::False;
    depth_stencil_state_create_info.stencilTestEnable = ::vk::False;
    depth_stencil_state_create_info.front = ::vk::StencilOpState{};
    depth_stencil_state_create_info.back = ::vk::StencilOpState{};
    depth_stencil_state_create_info.minDepthBounds = 0.0f;
    depth_stencil_state_create_info.maxDepthBounds = 0.0f;

    auto color_blend_attachment_states = std::array<::vk::PipelineColorBlendAttachmentState, 1>();
    for (std::size_t i = 0; i < color_blend_attachment_states.size(); ++i)
    {
        auto color_blend_attachment_state = ::vk::PipelineColorBlendAttachmentState{};
        switch (key.alpha_mode)
        {
            case AlphaMode::Blend:
            {
                color_blend_attachment_state.blendEnable = ::vk::True;
            }
            break;
            case AlphaMode::Mask:
            case AlphaMode::Opaque:
            {
                color_blend_attachment_state.blendEnable = ::vk::False;
            }
            break;
            default:
            {
                throw arm::Exception("unknown alpha mode");
            }
        }
        color_blend_attachment_state.srcColorBlendFactor = ::vk::BlendFactor::eSrcAlpha;
        color_blend_attachment_state.dstColorBlendFactor = ::vk::BlendFactor::eOneMinusSrcAlpha;
        color_blend_attachment_state.colorBlendOp = ::vk::BlendOp::eAdd;
        color_blend_attachment_state.srcAlphaBlendFactor = ::vk::BlendFactor::eOne;
        color_blend_attachment_state.dstAlphaBlendFactor = ::vk::BlendFactor::eZero;
        color_blend_attachment_state.alphaBlendOp = ::vk::BlendOp::eAdd;
        using enum ::vk::ColorComponentFlagBits;
        color_blend_attachment_state.colorWriteMask = eR | eG | eB | eA;

        color_blend_attachment_states[i] = std::move(color_blend_attachment_state);
    }

    auto color_blend_state_create_info = ::vk::PipelineColorBlendStateCreateInfo{};
    color_blend_state_create_info.sType = ::vk::StructureType::ePipelineColorBlendStateCreateInfo;
    color_blend_state_create_info.pNext = nullptr;
    color_blend_state_create_info.flags = {};
    color_blend_state_create_info.logicOpEnable = ::vk::False;
    color_blend_state_create_info.logicOp = ::vk::LogicOp::eCopy;
    color_blend_state_create_info.attachmentCount = static_cast<std::uint32_t>(color_blend_attachment_states.size());
    color_blend_state_create_info.pAttachments = color_blend_attachment_states.data();
    color_blend_state_create_info.blendConstants = {};

    auto dynamic_states = std::vector<::vk::DynamicState>{
        ::vk::DynamicState::eViewport,
        ::vk::DynamicState::eScissor,
    };
    auto dynamic_state_create_info = ::vk::PipelineDynamicStateCreateInfo{};
    dynamic_state_create_info.sType = ::vk::StructureType::ePipelineDynamicStateCreateInfo;
    dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state_create_info.pDynamicStates = dynamic_states.data();

    auto pipeline_rendering_create_info = ::vk::PipelineRenderingCreateInfo{};
    pipeline_rendering_create_info.sType = ::vk::StructureType::ePipelineRenderingCreateInfo;
    pipeline_rendering_create_info.pNext = nullptr;
    pipeline_rendering_create_info.viewMask = 0; // TODO magic-y number - must match renderer create info in renderer
    pipeline_rendering_create_info.colorAttachmentCount = static_cast<std::uint32_t>(color_attachment_formats_.size());
    pipeline_rendering_create_info.pColorAttachmentFormats = color_attachment_formats_.data();
    pipeline_rendering_create_info.depthAttachmentFormat = depth_format_;
    pipeline_rendering_create_info.stencilAttachmentFormat = stencil_format_;

    auto pipeline_create_info = ::vk::GraphicsPipelineCreateInfo{};
    pipeline_create_info.sType = ::vk::StructureType::eGraphicsPipelineCreateInfo;
    pipeline_create_info.pNext = &pipeline_rendering_create_info;
    pipeline_create_info.flags = {};
    pipeline_create_info.stageCount = static_cast<std::uint32_t>(shader_stages.size());
    pipeline_create_info.pStages = shader_stages.data();
    pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
    pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
    pipeline_create_info.pTessellationState = &tesselation_state_create_info;
    pipeline_create_info.pViewportState = &viewport_state_create_info;
    pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
    pipeline_create_info.pMultisampleState = &multisample_state_create_info;
    pipeline_create_info.pDepthStencilState = &depth_stencil_state_create_info;
    pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
    pipeline_create_info.pDynamicState = &dynamic_state_create_info;
    pipeline_create_info.layout = *pipeline_layout_;
    pipeline_create_info.renderPass = nullptr;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = nullptr;
    pipeline_create_info.basePipelineIndex = 0;

    auto result = check_vk_expected(device_.native_handle().createGraphicsPipeline(nullptr, pipeline_create_info));
    if (!result)
    {
        // TODO avoid exception once error handling is available
        throw arm::Exception("error: {} (\"{}\")", to_string(result.error().code), result.error().message);
    }

    return {
        name_pipeline(key),
        PassType::Main,
        key,
        vertex_shader_handle,
        fragment_shader_handle,
        std::move(result.value())};
}

} // namespace pong
