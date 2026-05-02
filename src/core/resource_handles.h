#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>

#include "utils/hash.h"

namespace pong
{

constexpr auto get_resource_id(std::string_view str) -> std::uint64_t
{
    return hash_string(str);
}

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

struct EntityIndex
{
    std::uint64_t value;
    auto operator<=>(const EntityIndex &other) const -> auto = default;
};

struct LightHandle
{
    std::uint32_t value;
    std::uint32_t version = 0u;
    auto operator==(const LightHandle &handle) const -> bool = default;
};

} // namespace pong

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
