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

    auto load(std::string name, const std::filesystem::path &path, ShaderStage stage) -> Shader &;
    auto load(Mesh mesh) -> Mesh &;

    template <typename T>
    auto get(this auto &&self, std::string_view name) -> auto &&;

    template <typename T>
    auto contains(std::string_view name) const -> bool;

  private:
    const VulkanDevice &device_;

    std::unordered_map<std::uint64_t, Shader> shaders_;
    std::unordered_map<std::uint64_t, Mesh> meshes_;

    template <typename T>
    auto get_map_(this auto &&self) -> auto &&;

    static constexpr auto get_resource_id_(std::string_view str) -> std::uint64_t
    {
        return pong::hash_string(str);
    }
};

template <typename T>
auto ResourceManager::get(this auto &&self, std::string_view name) -> auto &&
{
    auto &map = self.get_map_<T>();
    auto key = get_resource_id_(name);
    if (auto entry = map.find(key); entry != map.end())
    {
        return entry->second;
    }
    arm::ensure(false, "resource does not exist: {}", name);
    std::terminate();
}

template <typename T>
auto ResourceManager::contains(std::string_view name) const -> bool
{
    auto key = get_resource_id_(name);
    return get_map_<T>().contains(key);
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
        arm::ensure(!sizeof(T), "invalid resource type in ResourceManager::get_map_()");
        std::unreachable();
    }
}

}
