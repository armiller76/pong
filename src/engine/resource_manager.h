#pragma once

#include <filesystem>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "utils/error.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

class VulkanDevice;
class Mesh;
class Shader;
struct Vertex;

class ResourceManager
{
  public:
    explicit ResourceManager(const VulkanDevice &device);

    auto load(std::string_view name, const std::filesystem::path &path, ::vk::ShaderStageFlagBits stage) -> Shader &;
    auto load(std::string_view name, std::span<const Vertex> vertices) -> Mesh &;

    template <typename T>
    auto get(std::string_view name) -> T &
    {
        auto &map = get_map_<T>();
        if (auto entry = map.find(name); entry != map.end())
        {
            return entry->second;
        }
        arm::ensure(false, "resource does not exist: {}", name);
        std::terminate();
    }

    template <typename T>
    auto contains(std::string_view name) const -> bool
    {
        return get_map_<T>().contains(name);
    }

  private:
    const VulkanDevice &device_;

    template <typename V>
    using ResourceMap = std::unordered_map<std::string, V, std::hash<std::string_view>, std::equal_to<>>;
    ResourceMap<Shader> shaders_;
    ResourceMap<Mesh> meshes_;

    template <typename T>
    auto get_map_() -> ResourceMap<T> &
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
        }
    }
    template <typename T>
    auto get_map_() const -> const ResourceMap<T> &
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
        }
    }
};

}
