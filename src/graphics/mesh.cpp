#include "mesh.h"

#include <array>
#include <span>

#include "glm_wrapper.h"

#include "engine/vulkan/gpu_buffer.h"
#include "graphics/vertex.h"
#include "utils/error.h"

namespace pong
{

Mesh::Mesh(const VulkanDevice &device, std::span<const Vertex> vertices)
    : vertex_count_(static_cast<std::uint32_t>(vertices.size()))
    , vertex_buffer_(
          device,
          vertices.size_bytes(),
          ::vk::BufferUsageFlagBits::eVertexBuffer,
          ::vk::MemoryPropertyFlagBits::eHostCoherent | ::vk::MemoryPropertyFlagBits::eHostVisible)
{
    arm::ensure(!vertices.empty(), "cannot create Mesh with no vertices");
    arm::ensure(vertices.size() <= UINT32_MAX, "too many vertices (uint32_t max)");

    vertex_buffer_.upload(vertices.data(), vertices.size_bytes());

    arm::log::debug("Created Mesh: vertices={} bytes={}", vertex_count_, vertices.size_bytes());
}

auto Mesh::vertex_count() const noexcept -> std::uint32_t
{
    return vertex_count_;
}

auto Mesh::vertex_buffer() const noexcept -> const GpuBuffer &
{
    return vertex_buffer_;
}

auto Mesh::size_bytes() const noexcept -> std::size_t
{
    return vertex_count_ * sizeof(Vertex);
}

auto Mesh::create_test_triangle(const VulkanDevice &device) -> Mesh
{
    constexpr auto vertices = std::array<Vertex, 3>{
        {{.position = {0.0f, -0.5f}, .color = {1.0f, 0.0f, 0.0f}},
         {.position = {0.5f, 0.5f}, .color = {0.0f, 1.0f, 0.0f}},
         {.position = {-0.5f, 0.5f}, .color = {0.0f, 0.0f, 1.0f}}}};
    return Mesh(device, vertices);
}

auto Mesh::create_test_rectangle(const VulkanDevice &device) -> Mesh
{
    constexpr auto vertices = std::array<Vertex, 6>{
        {{.position = {-0.5f, -0.5f}, .color = {1.0f, 1.0f, 1.0f}},
         {.position = {0.5f, -0.5f}, .color = {1.0f, 1.0f, 1.0f}},
         {.position = {0.5f, 0.5f}, .color = {1.0f, 1.0f, 1.0f}},
         {.position = {0.5f, 0.5f}, .color = {1.0f, 1.0f, 1.0f}},
         {.position = {-0.5f, 0.5f}, .color = {1.0f, 1.0f, 1.0f}},
         {.position = {-0.5f, -0.5f}, .color = {1.0f, 1.0f, 1.0f}}}};
    return Mesh(device, vertices);
}

} // namespace pong
