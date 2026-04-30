#pragma once

#include <cstddef>

#include "glm_wrapper.h" // IWYU pragma: keep

namespace pong
{

struct Vertex
{
    ::glm::vec3 position;
    ::glm::vec4 color;
    ::glm::vec3 normal;
    ::glm::vec2 uv;


}; // struct Vertex

} // namespace pong

template <>
struct std::hash<pong::Vertex>
{
    auto operator()(const pong::Vertex &v) const noexcept -> std::size_t
    {
        auto result = std::size_t{42};
        const auto hash_combine = [&result](const std::size_t value)
        {
            // boost-style mix to preserve entropy from each field hash
            result ^= value + static_cast<std::size_t>(0x9e3779b97f4a7c15ull) + (result << 6u) + (result >> 2u);
        };
        hash_combine(std::hash<::glm::vec3>{}(v.position));
        hash_combine(std::hash<::glm::vec4>{}(v.color));
        hash_combine(std::hash<::glm::vec3>{}(v.normal));
        hash_combine(std::hash<::glm::vec2>{}(v.uv));
        hash_combine(std::hash<::glm::vec4>{}(v.tangent));
        return result;
    }
};
