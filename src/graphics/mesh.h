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
    Mesh(std::string name, const VulkanDevice &device, std::span<const Vertex> vertices);
    ~Mesh() = default;

    Mesh(Mesh &&) noexcept = default;
    Mesh &operator=(Mesh &&) noexcept = default;
    Mesh(const Mesh &) = delete;
    Mesh &operator=(Mesh &) = delete;

    auto vertex_count() const noexcept -> std::uint32_t;
    auto vertex_buffer() const noexcept -> const GpuBuffer &;
    auto size_bytes() const noexcept -> std::size_t;
    auto name() const noexcept -> std::string_view;

    static auto create_test_triangle(const VulkanDevice &device) -> Mesh;
    static auto create_test_rectangle(const VulkanDevice &device) -> Mesh;

  private:
    std::uint32_t vertex_count_{0};
    GpuBuffer vertex_buffer_;
    std::string name_{};
}; // class Vertex

} // namespace pong
