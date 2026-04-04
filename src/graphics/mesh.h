#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

#include "engine/vulkan/gpu_buffer.h"
#include "utils/log.h"
#include "vertex.h"

namespace pong
{
class VulkanDevice;

class Mesh
{
  public:
    Mesh(
        std::string_view name,
        const VulkanDevice &device,
        std::span<const Vertex> vertices,
        std::span<const std::uint32_t> indices);
    ~Mesh() = default;

    Mesh(Mesh &&) noexcept = default;
    Mesh &operator=(Mesh &&) noexcept = default;
    Mesh(const Mesh &) = delete;
    Mesh &operator=(Mesh &) = delete;

    auto vertex_count() const noexcept -> std::uint32_t;
    auto vertex_buffer() const noexcept -> const GpuBuffer &;
    auto vertices() const noexcept -> std::span<const Vertex>;

    auto index_count() const noexcept -> std::uint32_t;
    auto index_buffer() const noexcept -> const GpuBuffer &;
    auto indices() const noexcept -> std::span<const std::uint32_t>;

    auto name() const noexcept -> std::string_view;

    static auto create_test_triangle(const VulkanDevice &device) -> Mesh;
    static auto create_test_rectangle(const VulkanDevice &device) -> Mesh;

  private:
    std::vector<Vertex> vertices_cpu_{};
    std::vector<std::uint32_t> indices_cpu_{};
    GpuBuffer vertex_buffer_gpu_;
    GpuBuffer index_buffer_gpu_;
    std::string name_{};
}; // class Mesh

} // namespace pong
