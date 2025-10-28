#pragma once

#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "engine/vulkan/vulkan_utils.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "graphics/vertex.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace
{

constexpr auto get_resource_id(std::string_view str) -> std::uint64_t
{
    return pong::hash_string(str);
}

}

namespace pong
{

class VulkanDevice;

class ResourceManager
{
  public:
    explicit ResourceManager(const VulkanDevice &device);

    auto load(const std::string &name, const std::filesystem::path &path, ShaderStage stage) -> Shader &;
    auto load(const std::string &name, std::span<const Vertex> vertices) -> Mesh &;

    template <typename T>
    auto get(std::string_view name) -> T &
    {
        auto &map = get_map_<T>();
        auto key = get_resource_id(name);
        if (auto entry = map.find(key); entry != map.end())
        {
            return entry->second;
        }
        arm::ensure(false, "resource does not exist: {}", name);
        std::terminate();
    }

    template <typename T>
    auto contains(std::string_view name) const -> bool
    {
        auto key = get_resource_id(name);
        return get_map_<T>().contains(key);
    }

  private:
    const VulkanDevice &device_;

    std::unordered_map<std::uint64_t, Shader> shaders_;
    std::unordered_map<std::uint64_t, Mesh> meshes_;

    template <typename T>
    auto get_map_() -> std::unordered_map<std::uint64_t, T> &
    {
        if constexpr (std::is_same_v<T, Shader>)
        {
            return shaders_;
        }
        else if constexpr (std::is_same_v<T, Mesh>)
        {
            return meshes_;
        }
        else
        {
            arm::ensure(!sizeof(T), "invalid resource type in ResourceManager::get_map_()");
            std::unreachable();
        }
    }
    template <typename T>
    auto get_map_() const -> const std::unordered_map<std::uint64_t, T> &
    {
        if constexpr (std::is_same_v<T, Shader>)
        {
            return shaders_;
        }
        else if constexpr (std::is_same_v<T, Mesh>)
        {
            return meshes_;
        }
        else
        {
            arm::ensure(!sizeof(T), "invalid resource type in ResourceManager::get_map_() const");
            std::unreachable();
        }
    }
};

}
