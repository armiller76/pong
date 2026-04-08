#pragma once

#include <array>

#include <vulkan/vulkan_raii.hpp>

#include "glm_wrapper.h"

namespace pong
{

struct Vertex
{
    ::glm::vec3 position;
    ::glm::vec3 color;

    static auto get_binding_description() -> ::vk::VertexInputBindingDescription
    {
        auto result = ::vk::VertexInputBindingDescription{};
        result.binding = 0;
        result.stride = sizeof(Vertex);
        result.inputRate = ::vk::VertexInputRate::eVertex;
        return result;
    }

    static auto get_attribute_descriptions() -> std::array<::vk::VertexInputAttributeDescription, 2>
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

        return std::array<::vk::VertexInputAttributeDescription, 2>{{position_entry, color_entry}};
    }
};

}
