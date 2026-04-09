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
    ::glm::vec3 normal;
    ::glm::vec2 uv;

    static auto get_binding_description() -> ::vk::VertexInputBindingDescription
    {
        auto result = ::vk::VertexInputBindingDescription{};
        result.binding = 0;
        result.stride = sizeof(Vertex);
        result.inputRate = ::vk::VertexInputRate::eVertex;
        return result;
    }

    static auto get_attribute_descriptions() -> auto
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

        return std::array{
            position_entry,
            color_entry,
            normal_entry,
            texture_coordinate_entry,
        };
    }
};

}
