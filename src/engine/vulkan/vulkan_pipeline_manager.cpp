#include "engine/vulkan/vulkan_pipeline_manager.h"

#include <array>
#include <expected>
#include <utility>

#include <glm/mat4x4.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "core/resource_handles.h"
#include "engine/engine_types.h"
#include "engine/resource_manager.h"
#include "engine/vulkan/vulkan_descriptor_pool.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_pipeline_types.h"
#include "engine/vulkan/vulkan_render_utils.h"
#include "graphics/shader.h"
#include "graphics/types.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

VulkanPipelineManager::VulkanPipelineManager(
    const VulkanDevice &device,
    VulkanDescriptorPool &descriptor_pool,
    ResourceManager &resource_manager)
    : device_{device}
    , resource_manager_{resource_manager}
    , descriptor_pool_{descriptor_pool}
    , set_0_per_frame_layout_{create_per_frame_descriptor_set_layout_()}
    , set_1_per_material_layout_{create_per_material_descriptor_set_layout_()}
    , push_constant_ranges_{create_push_constant_ranges_()}
    , depth_format_{device_.choose_depth_format()}
    , stencil_format_{::vk::Format::eUndefined} // TODO magic number
    , pipeline_layout_{create_layout_()}
{
    arm::log::debug("VulkanPipelineManager constructor");
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

auto VulkanPipelineManager::get_pipeline_layout() const -> const ::vk::raii::PipelineLayout &
{
    return pipeline_layout_;
}

auto VulkanPipelineManager::get_default_pipeline_key() const -> PipelineKey
{
    return {
        .pass_type = PassType::Main,
        .alpha_mode = AlphaMode::Opaque,
        .raster_state = RasterState::Default,
        .vertex_input = VertexInput::NOT_IMPLEMENTED,
        .shader_features = ShaderFeature{},
    };
}

auto VulkanPipelineManager::get_per_frame_descriptor_set_layout() const -> const ::vk::raii::DescriptorSetLayout &
{
    return set_0_per_frame_layout_;
}

auto VulkanPipelineManager::allocate_material_descriptor_set() -> ::vk::raii::DescriptorSet
{
    return descriptor_pool_.allocate_material_descriptor_set(set_1_per_material_layout_);
}

auto VulkanPipelineManager::set_color_attachment_format(::vk::Format format) -> void
{
    if (format == ::vk::Format::eUndefined)
    {
        throw arm::Exception("invalid format");
    }
    if (!color_attachment_formats_.empty() && color_attachment_formats_[0] != ::vk::Format::eUndefined)
    {
        arm::log::error("max color attachment formats currently {}, overwriting", color_attachment_formats_.size());
    }
    color_attachment_formats_ = std::array{
        format,
    };
}

auto VulkanPipelineManager::create_per_frame_descriptor_set_layout_() -> ::vk::raii::DescriptorSetLayout
{
    // ---- SET 0 ---- //
    const auto view_proj_ubo_layout_binding = ::vk::DescriptorSetLayoutBinding{
        .binding = 0u,
        .descriptorType = ::vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1u,
        .stageFlags = ::vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr,
    };
    const auto light_ubo_layout_binding = ::vk::DescriptorSetLayoutBinding{
        .binding = 1u,
        .descriptorType = ::vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1u,
        .stageFlags = ::vk::ShaderStageFlagBits::eFragment,
        .pImmutableSamplers = nullptr,
    };
    const auto per_frame_layout_bindings = std::vector{
        view_proj_ubo_layout_binding,
        light_ubo_layout_binding,
    };

    // ---- SET 0 ---- //
    const auto per_frame_descriptor_set_layout_create_info = ::vk::DescriptorSetLayoutCreateInfo{
        .sType = ::vk::StructureType::eDescriptorSetLayoutCreateInfo,
        .pNext = nullptr,
        .flags = {},
        .bindingCount = static_cast<std::uint32_t>(per_frame_layout_bindings.size()),
        .pBindings = per_frame_layout_bindings.data(),
    };

    // ---- SET 0 ---- //

    auto result =
        check_vk_expected(device_.get().createDescriptorSetLayout(per_frame_descriptor_set_layout_create_info));
    if (!result.has_value())
    {
        // TODO avoid exception once error handling is available
        throw arm::Exception("error: {} (\"{}\")", to_string(result.error().code), result.error().message);
    }

    return std::move(result.value());
}

auto VulkanPipelineManager::create_per_material_descriptor_set_layout_() -> ::vk::raii::DescriptorSetLayout
{
    // ---- SET 1 ---- //
    const auto base_sampler_layout_binding = ::vk::DescriptorSetLayoutBinding{
        .binding = 0u,
        .descriptorType = ::vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = 1u,
        .stageFlags = ::vk::ShaderStageFlagBits::eFragment,
        .pImmutableSamplers = nullptr,
    };
    const auto metal_sampler_layout_binding = ::vk::DescriptorSetLayoutBinding{
        .binding = 1u,
        .descriptorType = ::vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = 1u,
        .stageFlags = ::vk::ShaderStageFlagBits::eFragment,
        .pImmutableSamplers = nullptr,
    };
    const auto normal_sampler_layout_binding = ::vk::DescriptorSetLayoutBinding{
        .binding = 2u,
        .descriptorType = ::vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = 1u,
        .stageFlags = ::vk::ShaderStageFlagBits::eFragment,
        .pImmutableSamplers = nullptr,
    };
    const auto material_ubo_layout_binding = ::vk::DescriptorSetLayoutBinding{
        .binding = 3u,
        .descriptorType = ::vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1u,
        .stageFlags = ::vk::ShaderStageFlagBits::eFragment,
        .pImmutableSamplers = nullptr,
    };
    const auto per_material_layout_bindings = std::vector{
        base_sampler_layout_binding,
        metal_sampler_layout_binding,
        normal_sampler_layout_binding,
        material_ubo_layout_binding,
    };

    // ---- SET 1 ---- //
    const auto per_material_descriptor_set_layout_create_info = ::vk::DescriptorSetLayoutCreateInfo{
        .sType = ::vk::StructureType::eDescriptorSetLayoutCreateInfo,
        .pNext = nullptr,
        .flags = {},
        .bindingCount = static_cast<std::uint32_t>(per_material_layout_bindings.size()),
        .pBindings = per_material_layout_bindings.data(),
    };

    // ---- SET 1 ---- //
    auto result =
        check_vk_expected(device_.get().createDescriptorSetLayout(per_material_descriptor_set_layout_create_info));
    if (!result.has_value())
    {
        // TODO avoid exception once error handling is available
        throw arm::Exception("error: {} (\"{}\")", to_string(result.error().code), result.error().message);
    }

    return std::move(result.value());
}

auto VulkanPipelineManager::create_push_constant_ranges_() -> std::array<::vk::PushConstantRange, 1>
{
    const auto model_push_constant_range = ::vk::PushConstantRange{
        .stageFlags = ::vk::ShaderStageFlagBits::eVertex,
        .offset = 0,
        .size = sizeof(::glm::mat4),
    };

    return std::array{
        model_push_constant_range,
    };
}

auto VulkanPipelineManager::create_layout_() -> ::vk::raii::PipelineLayout
{
    auto pipeline_descriptor_set_layouts = std::vector{
        *set_0_per_frame_layout_,
        *set_1_per_material_layout_,
    };

    const auto pipeline_layout_create_info = ::vk::PipelineLayoutCreateInfo{
        .sType = ::vk::StructureType::ePipelineLayoutCreateInfo,
        .pNext = nullptr,
        .flags = {},
        .setLayoutCount = static_cast<std::uint32_t>(pipeline_descriptor_set_layouts.size()),
        .pSetLayouts = pipeline_descriptor_set_layouts.data(),
        .pushConstantRangeCount = static_cast<std::uint32_t>(push_constant_ranges_.size()),
        .pPushConstantRanges = push_constant_ranges_.data(),
    };

    auto result = check_vk_expected(device_.get().createPipelineLayout(pipeline_layout_create_info));
    if (!result.has_value())
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
    const auto &vertex_shader = resource_manager_.get<Shader>(vertex_shader_handle);
    const auto vertex_stage_create_info = ::vk::PipelineShaderStageCreateInfo{
        .sType = ::vk::StructureType::ePipelineShaderStageCreateInfo,
        .pNext = nullptr,
        .flags = {},
        .stage = ::vk::ShaderStageFlagBits::eVertex,
        .module = vertex_shader.module_handle(),
        .pName = vertex_shader.entry_point_c_str(),
        .pSpecializationInfo = nullptr,
    };

    const auto &fragment_shader = resource_manager_.get<Shader>(fragment_shader_handle);
    const auto fragment_stage_create_info = ::vk::PipelineShaderStageCreateInfo{
        .sType = ::vk::StructureType::ePipelineShaderStageCreateInfo,
        .pNext = nullptr,
        .flags = {},
        .stage = ::vk::ShaderStageFlagBits::eFragment,
        .module = fragment_shader.module_handle(),
        .pName = fragment_shader.entry_point_c_str(),
        .pSpecializationInfo = nullptr,
    };

    const auto shader_stages = std::array{
        vertex_stage_create_info,
        fragment_stage_create_info,
    };

    const auto vertex_input_binding_description = get_vertex_input_binding_description();
    const auto vertex_input_attribute_descriptions = get_vertex_input_attribute_descriptions();
    const auto vertex_input_state_create_info = ::vk::PipelineVertexInputStateCreateInfo{
        .sType = ::vk::StructureType::ePipelineVertexInputStateCreateInfo,
        .pNext = nullptr,
        .flags = {},
        .vertexBindingDescriptionCount = 1, // TODO magic(ish) number
        .pVertexBindingDescriptions = &vertex_input_binding_description,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_input_attribute_descriptions.size()),
        .pVertexAttributeDescriptions = vertex_input_attribute_descriptions.data(),
    };

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

    const auto tesselation_state_create_info = ::vk::PipelineTessellationStateCreateInfo{
        .sType = ::vk::StructureType::ePipelineTessellationStateCreateInfo,
        .pNext = nullptr,
        .flags = {},
        .patchControlPoints = 0,
    };

    //  Viewport and Scissor are dynamic, viewport and scissor pointers are ignored
    const auto viewport_state_create_info = ::vk::PipelineViewportStateCreateInfo{
        .sType = ::vk::StructureType::ePipelineViewportStateCreateInfo,
        .pNext = nullptr,
        .flags = {},
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };

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

    const auto multisample_state_create_info = ::vk::PipelineMultisampleStateCreateInfo{
        .sType = ::vk::StructureType::ePipelineMultisampleStateCreateInfo,
        .pNext = nullptr,
        .flags = {},
        .rasterizationSamples = ::vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = ::vk::False,
        .minSampleShading = 0.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = ::vk::False,
        .alphaToOneEnable = ::vk::False,
    };

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
    for (auto &state : color_blend_attachment_states)
    {
        switch (key.alpha_mode)
        {
            case AlphaMode::Blend:
            {
                state.blendEnable = ::vk::True;
            }
            break;
            case AlphaMode::Mask:
            case AlphaMode::Opaque:
            {
                state.blendEnable = ::vk::False;
            }
            break;
            default:
            {
                throw arm::Exception("unknown alpha mode");
            }
        }
        state.srcColorBlendFactor = ::vk::BlendFactor::eSrcAlpha;
        state.dstColorBlendFactor = ::vk::BlendFactor::eOneMinusSrcAlpha;
        state.colorBlendOp = ::vk::BlendOp::eAdd;
        state.srcAlphaBlendFactor = ::vk::BlendFactor::eOne;
        state.dstAlphaBlendFactor = ::vk::BlendFactor::eZero;
        state.alphaBlendOp = ::vk::BlendOp::eAdd;
        using enum ::vk::ColorComponentFlagBits;
        state.colorWriteMask = eR | eG | eB | eA;
    }

    const auto color_blend_state_create_info = ::vk::PipelineColorBlendStateCreateInfo{
        .sType = ::vk::StructureType::ePipelineColorBlendStateCreateInfo,
        .pNext = nullptr,
        .flags = {},
        .logicOpEnable = ::vk::False,
        .logicOp = ::vk::LogicOp::eCopy,
        .attachmentCount = static_cast<std::uint32_t>(color_blend_attachment_states.size()),
        .pAttachments = color_blend_attachment_states.data(),
        .blendConstants = {},
    };

    const auto dynamic_states = std::vector<::vk::DynamicState>{
        ::vk::DynamicState::eViewport,
        ::vk::DynamicState::eScissor,
    };
    const auto dynamic_state_create_info = ::vk::PipelineDynamicStateCreateInfo{
        .sType = ::vk::StructureType::ePipelineDynamicStateCreateInfo,
        .pNext = nullptr,
        .flags = {},
        .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
        .pDynamicStates = dynamic_states.data(),
    };

    const auto pipeline_rendering_create_info = ::vk::PipelineRenderingCreateInfo{
        .sType = ::vk::StructureType::ePipelineRenderingCreateInfo,
        .pNext = nullptr,
        .viewMask = 0, // TODO magic-y number - must match renderer create info in renderer
        .colorAttachmentCount = static_cast<std::uint32_t>(color_attachment_formats_.size()),
        .pColorAttachmentFormats = color_attachment_formats_.data(),
        .depthAttachmentFormat = depth_format_,
        .stencilAttachmentFormat = stencil_format_,
    };

    const auto pipeline_create_info = ::vk::GraphicsPipelineCreateInfo{
        .sType = ::vk::StructureType::eGraphicsPipelineCreateInfo,
        .pNext = &pipeline_rendering_create_info,
        .flags = {},
        .stageCount = static_cast<std::uint32_t>(shader_stages.size()),
        .pStages = shader_stages.data(),
        .pVertexInputState = &vertex_input_state_create_info,
        .pInputAssemblyState = &input_assembly_state_create_info,
        .pTessellationState = &tesselation_state_create_info,
        .pViewportState = &viewport_state_create_info,
        .pRasterizationState = &rasterization_state_create_info,
        .pMultisampleState = &multisample_state_create_info,
        .pDepthStencilState = &depth_stencil_state_create_info,
        .pColorBlendState = &color_blend_state_create_info,
        .pDynamicState = &dynamic_state_create_info,
        .layout = *pipeline_layout_,
        .renderPass = nullptr,
        .subpass = 0,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = 0,
    };

    auto pipeline_result = check_vk_expected(device_.get().createGraphicsPipeline(nullptr, pipeline_create_info));
    if (!pipeline_result.has_value())
    {
        // TODO avoid exception once error handling is available
        throw arm::Exception(
            "error: {} (\"{}\")", to_string(pipeline_result.error().code), pipeline_result.error().message);
    }

    return {
        .name = name_pipeline(key),
        .pass_type = PassType::Main,
        .pipeline_key = key,
        .vertex_shader = vertex_shader_handle,
        .fragment_shader = fragment_shader_handle,
        .pipeline = std::move(pipeline_result.value()),
    };
}

} // namespace pong
