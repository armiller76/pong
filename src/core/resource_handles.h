#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace pong
{

struct ShaderHandle
{
    std::uint64_t value;
    auto operator<=>(const ShaderHandle &other) const -> auto = default;
};

struct MeshHandle
{
    std::uint64_t value;
    auto operator<=>(const MeshHandle &other) const -> auto = default;
};

struct Texture2DHandle
{
    std::uint64_t value;
    auto operator<=>(const Texture2DHandle &other) const -> auto = default;
};

struct MaterialHandle
{
    std::uint64_t value;
    auto operator<=>(const MaterialHandle &other) const -> auto = default;
};

struct ModelHandle
{
    std::uint64_t value;
    auto operator<=>(const ModelHandle &other) const -> auto = default;
};

// struct ImageHandle
//{
//     std::uint64_t value;
//     auto operator==(const ImageHandle &other) const -> bool = default;
// };

}

template <>
struct std::hash<pong::ShaderHandle>
{
    auto operator()(const pong::ShaderHandle &obj) const noexcept -> std::size_t
    {
        return std::hash<std::uint64_t>{}(obj.value);
    }
};

template <>
struct std::hash<pong::MeshHandle>
{
    auto operator()(const pong::MeshHandle &obj) const noexcept -> std::size_t
    {
        return std::hash<std::uint64_t>{}(obj.value);
    }
};

template <>
struct std::hash<pong::Texture2DHandle>
{
    auto operator()(const pong::Texture2DHandle &obj) const noexcept -> std::size_t
    {
        return std::hash<std::uint64_t>{}(obj.value);
    }
};

template <>
struct std::hash<pong::MaterialHandle>
{
    auto operator()(const pong::MaterialHandle &obj) const noexcept -> std::size_t
    {
        return std::hash<std::uint64_t>{}(obj.value);
    }
};

template <>
struct std::hash<pong::ModelHandle>
{
    auto operator()(const pong::ModelHandle &obj) const noexcept -> std::size_t
    {
        return std::hash<std::uint64_t>{}(obj.value);
    }
};

// template <>
// struct std::hash<pong::ImageHandle>
//{
//     auto operator()(const pong::ImageHandle &obj) const noexcept -> std::size_t
//     {
//         return std::hash<std::uint64_t>{}(obj.value);
//     }
// };
