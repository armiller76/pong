#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

#include "core/resource_handles.h"
#include "engine/vulkan/vulkan_immediate_command_context.h"
#include "engine/vulkan/vulkan_utils.h"
#include "graphics/image.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "graphics/texture2d.h"
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
    auto operator=(const ResourceManager &) -> ResourceManager & = delete;
    ResourceManager(ResourceManager &&) noexcept = delete;
    auto operator=(ResourceManager &&) noexcept -> ResourceManager & = delete;

    // Shader loading
    auto load(std::string_view name, const std::filesystem::path &path, ShaderStage stage) -> ShaderHandle;

    // Mesh loading
    auto load(Mesh &&mesh) -> MeshHandle;
    auto load(std::string_view name, const std::filesystem::path &path) -> MeshHandle;

    // Texture loading
    auto load(Texture2D &&texture) -> Texture2DHandle;
    auto load(std::string_view name, Image &image) -> Texture2DHandle;

    template <typename T, typename R>
    auto unload(R resource_id);

    template <typename T, typename R>
    auto get(this auto &&self, R resource_id) -> auto &&;

    template <typename T, typename Q>
    auto contains(Q query) const -> bool;

  private:
    const VulkanDevice &device_;
    std::unordered_map<ShaderHandle, Shader> shaders_;
    std::unordered_map<MeshHandle, Mesh> meshes_;
    std::unordered_map<Texture2DHandle, Texture2D> textures_;
    VulkanImmediateCommandContext command_context_;

    template <typename T>
    auto get_map_(this auto &&self) -> auto &&;

    static constexpr auto get_resource_id_(std::string_view str) -> std::uint64_t;
};

template <typename T, typename R>
auto ResourceManager::unload(R resource_id)
{
    // TODO potential future bug: are you checking whether a resource is in-flight before removing from map?
    auto &map = get_map_<T>();
    arm::ensure(map.contains(resource_id), "ResourceManager does not contain resource id {}", resource_id);

    // check if any entities are using the resource_id being deleted? or maybe add ref counting to resources
    // managed by resourcemanager?
    map.erase(resource_id);
}

template <typename T, typename R>
auto ResourceManager::get(this auto &&self, R resource_id) -> auto &&
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
        // a string or string_view was passed, query is by resource name, so get the actual resource id first
        auto key = get_resource_id_(query);
        return map.contains(key);
    }
    else if constexpr (std::is_same_v<Q, std::uint64_t>)
    {
        // an actual resource_id was passed
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
    else if constexpr (std::is_same_v<T, Texture2D>)
    {
        return self.textures_;
    }
    else
    {
        static_assert(sizeof(T) == 0, "unsupported resource type");
        std::unreachable();
    }
}

constexpr auto ResourceManager::get_resource_id_(std::string_view str) -> std::uint64_t
{
    return pong::hash_string(str);
}

} // namespace pong
