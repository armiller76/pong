#pragma once

#include <array>

#include <vulkan/vulkan_raii.hpp>

#include "glm_wrapper.h"

namespace pong
{

struct Vertex
{
    ::glm::vec2 position;
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
        auto pos_entry = ::vk::VertexInputAttributeDescription{};
        pos_entry.location = 0;
        pos_entry.binding = 0;
        pos_entry.format = ::vk::Format::eR32G32Sfloat;
        pos_entry.offset = offsetof(Vertex, position);
        auto col_entry = ::vk::VertexInputAttributeDescription{};
        col_entry.location = 1;
        col_entry.binding = 0;
        col_entry.format = ::vk::Format::eR32G32B32Sfloat;
        col_entry.offset = offsetof(Vertex, color);

        return std::array<::vk::VertexInputAttributeDescription, 2>(pos_entry, col_entry);
    }
};

}
