#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_image_transition_info.h"
#include "graphics/vertex.h"

namespace pong
{

auto transition(
    ::vk::raii::CommandBuffer &command_buffer,
    ::vk::Image image,
    ::vk::ImageAspectFlags aspect_flags,
    VulkanImageTransitionInfo info) -> void
{
    const auto barrier = ::vk::ImageMemoryBarrier2{
        .sType = ::vk::StructureType::eImageMemoryBarrier2,
        .srcStageMask = info.src_stage,
        .srcAccessMask = info.src_access,
        .dstStageMask = info.dst_stage,
        .dstAccessMask = info.dst_access,
        .oldLayout = info.src_layout,
        .newLayout = info.dst_layout,
        // TODO when do we stop ignoring queue family index?
        .srcQueueFamilyIndex = ::vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = ::vk::QueueFamilyIgnored,
        .image = image,
        .subresourceRange{
            .aspectMask = aspect_flags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    const auto dependency_info = ::vk::DependencyInfo{
        .sType = ::vk::StructureType::eDependencyInfo,
        .pNext = nullptr,
        .dependencyFlags = {},
        .memoryBarrierCount = 0u,
        .pMemoryBarriers = nullptr,
        .bufferMemoryBarrierCount = 0u,
        .pBufferMemoryBarriers = nullptr,
        .imageMemoryBarrierCount = 1u,
        .pImageMemoryBarriers = &barrier,
    };

    command_buffer.pipelineBarrier2(dependency_info);
}

auto make_color_attachment(::vk::ImageView image_view, ::vk::ClearColorValue clear_color)
    -> ::vk::RenderingAttachmentInfo
{
    return {
        .sType = ::vk::StructureType::eRenderingAttachmentInfo,
        .pNext = nullptr,
        .imageView = image_view,
        .imageLayout = ::vk::ImageLayout::eColorAttachmentOptimal,
        .resolveMode = ::vk::ResolveModeFlagBits::eNone,
        .resolveImageView = nullptr,
        .resolveImageLayout = ::vk::ImageLayout::eUndefined,
        .loadOp = ::vk::AttachmentLoadOp::eClear,
        .storeOp = ::vk::AttachmentStoreOp::eStore,
        .clearValue = clear_color,
    };
}

auto make_depth_attachment(::vk::ImageView image_view) -> ::vk::RenderingAttachmentInfo
{
    return {
        .sType = ::vk::StructureType::eRenderingAttachmentInfo,
        .pNext = nullptr,
        .imageView = image_view,
        .imageLayout = ::vk::ImageLayout::eDepthStencilAttachmentOptimal,
        .resolveMode = ::vk::ResolveModeFlagBits::eNone,
        .resolveImageView = nullptr,
        .resolveImageLayout = ::vk::ImageLayout::eUndefined,
        .loadOp = ::vk::AttachmentLoadOp::eClear,
        .storeOp = ::vk::AttachmentStoreOp::eDontCare,
        .clearValue =
            ::vk::ClearDepthStencilValue{
                .depth = 1.0f,
                .stencil = 0u,
            },
    };
}

auto get_vertex_input_binding_description() -> ::vk::VertexInputBindingDescription
{
    return {
        .binding = 0,
        .stride = sizeof(pong::Vertex),
        .inputRate = ::vk::VertexInputRate::eVertex,
    };
}

auto get_vertex_input_attribute_descriptions() -> std::vector<::vk::VertexInputAttributeDescription>
{
    const auto position_entry = ::vk::VertexInputAttributeDescription{
        .location = 0,
        .binding = 0,
        .format = ::vk::Format::eR32G32B32Sfloat,
        .offset = offsetof(pong::Vertex, position),
    };
    const auto color_entry = ::vk::VertexInputAttributeDescription{
        .location = 1,
        .binding = 0,
        .format = ::vk::Format::eR32G32B32Sfloat,
        .offset = offsetof(pong::Vertex, color),
    };
    const auto normal_entry = ::vk::VertexInputAttributeDescription{
        .location = 2,
        .binding = 0,
        .format = ::vk::Format::eR32G32B32Sfloat,
        .offset = offsetof(pong::Vertex, normal),
    };
    const auto texture_coordinate_entry = ::vk::VertexInputAttributeDescription{
        .location = 3,
        .binding = 0,
        .format = ::vk::Format::eR32G32Sfloat,
        .offset = offsetof(pong::Vertex, uv),
    };
    const auto tangent_entry = ::vk::VertexInputAttributeDescription{
        .location = 4,
        .binding = 0,
        .format = ::vk::Format::eR32G32B32A32Sfloat,
        .offset = offsetof(pong::Vertex, tangent),
    };

    return std::vector{
        position_entry,
        color_entry,
        normal_entry,
        texture_coordinate_entry,
        tangent_entry,
    };
}

} // namespace pong
