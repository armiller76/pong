#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

#include "engine/vulkan/vulkan_utils.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "utils/error.h"

namespace pong
{

class VulkanDevice;

class ResourceManager
{
  public:
    explicit ResourceManager(const VulkanDevice &device);
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager &) = delete;
    ResourceManager &operator=(const ResourceManager &) = delete;
    ResourceManager(ResourceManager &&) = delete;
    ResourceManager &operator=(ResourceManager &&) = delete;

    auto load(std::string name, const std::filesystem::path &path, ShaderStage stage) -> std::uint64_t;
    auto load(Mesh &&mesh) -> std::uint64_t;

    template <typename T>
    auto get(this auto &&self, std::uint64_t resource_id) -> auto &&;

    template <typename T, typename Q>
    auto contains(Q query) const -> bool;

    static constexpr auto get_resource_id(std::string_view str) -> std::uint64_t;

  private:
    const VulkanDevice &device_;

    std::unordered_map<std::uint64_t, Shader> shaders_;
    std::unordered_map<std::uint64_t, Mesh> meshes_;

    template <typename T>
    auto get_map_(this auto &&self) -> auto &&;
};

template <typename T>
auto ResourceManager::get(this auto &&self, std::uint64_t resource_id) -> auto &&
{
    auto &map = self.get_map_<T>();
    if (auto entry = map.find(resource_id); entry != map.end())
    {
        return entry->second;
    }
    arm::ensure(false, "resource_id not found");
    std::unreachable();
}

template <typename T, typename Q>
auto ResourceManager::contains(Q query) const -> bool
{
    auto &map = get_map_<T>();
    if constexpr (std::is_convertible_v<Q, std::string_view>)
    {
        auto key = get_resource_id(query);
        return map.contains(key);
    }
    else if constexpr (std::is_same_v<Q, std::uint64_t>)
    {
        return map.contains(query);
    }
    else
    {
        static_assert(sizeof(T) == 0, "unsupported resource type");
        std::unreachable();
    }
}

template <typename T>
auto ResourceManager::get_map_(this auto &&self) -> auto &&
{
    if constexpr (std::is_same_v<T, Shader>)
    {
        return self.shaders_;
    }
    else if constexpr (std::is_same_v<T, Mesh>)
    {
        return self.meshes_;
    }
    else
    {
        static_assert(sizeof(T) == 0, "unsupported resource type");
        std::unreachable();
    }
}

constexpr auto ResourceManager::get_resource_id(std::string_view str) -> std::uint64_t
{
    return pong::hash_string(str);
}

} // namespace pong
