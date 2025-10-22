#pragma once

#include <array>

#include <vulkan/vulkan_raii.hpp>

namespace pong
{

struct Vertex
{
    float x;
    float y;
    // float z;
    // float w;
    float r;
    float g;
    float b;

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
        pos_entry.offset = offsetof(Vertex, x);
        auto col_entry = ::vk::VertexInputAttributeDescription{};
        col_entry.location = 1;
        col_entry.binding = 0;
        col_entry.format = ::vk::Format::eR32G32B32Sfloat;
        col_entry.offset = offsetof(Vertex, r);

        return std::array<::vk::VertexInputAttributeDescription, 2>(pos_entry, col_entry);
    }
};

}
