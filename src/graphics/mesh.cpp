#include "mesh.h"

#include <array>
#include <format>
#include <span>
#include <string>
#include <string_view>

#include "glm_wrapper.h"

#include "engine/vulkan/gpu_buffer.h"
#include "graphics/vertex.h"
#include "utils/error.h"

namespace pong
{

Mesh::Mesh(
    std::string_view name,
    const VulkanDevice &device,
    std::span<const Vertex> vertices,
    std::span<const std::uint32_t> indices)
    : vertices_cpu_{vertices.begin(), vertices.end()}
    , indices_cpu_{indices.begin(), indices.end()}
    , vertex_buffer_gpu_{device, vertices.size_bytes(), ::vk::BufferUsageFlagBits::eVertexBuffer, ::vk::MemoryPropertyFlagBits::eHostCoherent | ::vk::MemoryPropertyFlagBits::eHostVisible}
    , index_buffer_gpu_{device, indices.size_bytes(), ::vk::BufferUsageFlagBits::eIndexBuffer, ::vk::MemoryPropertyFlagBits::eHostCoherent | ::vk::MemoryPropertyFlagBits::eHostVisible}
    , name_{std::format("mesh_{}", name)}
{
    arm::ensure(!vertices.empty(), "cannot create Mesh with no vertices");
    arm::ensure(vertices.size() <= UINT32_MAX, "too many vertices (uint32_t max)");
    vertex_buffer_gpu_.upload(vertices.data(), vertices.size_bytes());

    arm::ensure(!indices.empty(), "cannot create Mesh with no indices");
    arm::ensure(indices.size() <= UINT32_MAX, "too many indices (uint32_t max)");
    index_buffer_gpu_.upload(indices.data(), indices.size_bytes());

    arm::log::debug("Created Mesh: vertices={} indices={}", vertices_cpu_.size(), indices_cpu_.size());
}

auto Mesh::vertex_count() const noexcept -> std::uint32_t
{
    return static_cast<uint32_t>(vertices_cpu_.size());
}

auto Mesh::vertex_buffer() const noexcept -> const GpuBuffer &
{
    return vertex_buffer_gpu_;
}

auto Mesh::vertices() const noexcept -> std::span<const Vertex>
{
    return vertices_cpu_;
}

auto Mesh::index_count() const noexcept -> std::uint32_t
{
    return static_cast<uint32_t>(indices_cpu_.size());
}

auto Mesh::index_buffer() const noexcept -> const GpuBuffer &
{
    return index_buffer_gpu_;
}

auto Mesh::indices() const noexcept -> std::span<const std::uint32_t>
{
    return indices_cpu_;
}

auto Mesh::name() const noexcept -> std::string_view
{
    return name_;
}

auto Mesh::create_test_triangle(const VulkanDevice &device) -> Mesh
{
    const std::vector<Vertex> vertices = {
        {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    };

    const std::vector<std::uint32_t> indices = {
        0,
        2,
        1,
    };

    return Mesh("test_triangle", device, vertices, indices);
}

auto Mesh::create_test_rectangle(const VulkanDevice &device) -> Mesh
{
    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 1.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    };

    const std::vector<std::uint32_t> indices = {
        0,
        3,
        1, // first triangle
        1,
        3,
        2, // second triangle
    };

    return Mesh("test_rectangle", device, vertices, indices);
}

} // namespace pong
