#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_image_transition_info.h"

namespace pong
{

auto transition(
    ::vk::raii::CommandBuffer &command_buffer,
    ::vk::Image image,
    ::vk::ImageAspectFlags aspect_flags,
    VulkanImageTransitionInfo info) -> void;

auto make_color_attachment(::vk::ImageView image_view, ::vk::ClearColorValue clear_color)
    -> ::vk::RenderingAttachmentInfo;

auto make_depth_attachment(::vk::ImageView image_view) -> ::vk::RenderingAttachmentInfo;

auto get_vertex_input_binding_description() -> ::vk::VertexInputBindingDescription;

auto get_vertex_input_attribute_descriptions() -> std::vector<::vk::VertexInputAttributeDescription>;

} // namespace pong
