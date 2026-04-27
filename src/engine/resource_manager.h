#pragma once

#include <string_view>
#include <unordered_map>
#include <utility>

#include "core/resource_handles.h"
#include "core/resource_traits.h"
#include "graphics/material.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "graphics/texture2d.h"
#include "utils/error.h"
#include "utils/hash.h"

namespace pong
{

class VulkanDevice;

class ResourceManager
{
  public:
    explicit ResourceManager()
        : default_vertex_shader_handle_{INVALID_RESOURCE_ID}
        , default_fragment_shader_handle_{INVALID_RESOURCE_ID}
    {
        arm::log::debug("ResourceManager constructor");
    }
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager &) = delete;
    auto operator=(const ResourceManager &) -> ResourceManager & = delete;
    ResourceManager(ResourceManager &&) noexcept = delete;
    auto operator=(ResourceManager &&) noexcept -> ResourceManager & = delete;

    auto shutdown() -> void
    {
        materials_.clear();
    }

    template <typename ResourceType>
    auto insert(std::string_view name, ResourceType &&obj) ->
        typename ResourceTraits<std::remove_cvref_t<ResourceType>>::handle_type
    {
        using T = std::remove_cvref_t<ResourceType>;
        const auto key = typename ResourceTraits<T>::handle_type{get_resource_id(name)};
        auto &map = get_map_<T>();

        auto [entry, inserted] = map.try_emplace(key, std::move(obj));
        if (!inserted)
        {
            arm::log::warn("resource already loaded: {} (skipping)", name);
        }
        return key;
    }

    template <typename T>
    auto unload(typename ResourceTraits<T>::handle_type resource_handle) -> void
    {
        // TODO potential future bug: are you checking whether a resource is in-flight before removing from map?
        auto &map = get_map_<T>();
        arm::ensure(
            map.contains(resource_handle), "ResourceManager does not contain resource id {}", resource_handle.value);

        // check if any entities are using the resource_id being deleted? or maybe add ref counting to resources
        // managed by resourcemanager?
        map.erase(resource_handle);
    }

    template <typename T>
    auto get(this auto &&self, typename ResourceTraits<T>::handle_type resource_handle) -> auto &&
    {
        auto &map = self.template get_map_<T>();
        if (auto entry = map.find(resource_handle); entry != map.end())
        {
            return entry->second;
        }
        arm::ensure(false, "resource_id {} not found", resource_handle.value);
        std::unreachable();
    }

    template <typename ResourceType, typename QueryType>
    auto contains(QueryType query) const -> bool
    {
        auto &map = get_map_<ResourceType>();
        if constexpr (std::is_convertible_v<QueryType, std::string_view>)
        {
            auto key = typename ResourceTraits<ResourceType>::handle_type{get_resource_id(query)};
            return map.contains(key);
        }
        else if constexpr (std::is_same_v<QueryType, typename ResourceTraits<ResourceType>::handle_type>)
        {
            return map.contains(query);
        }
        else
        {
            static_assert(sizeof(ResourceType) == 0, "unsupported resource type");
            std::unreachable();
        }
    }

    auto default_vertex_shader() -> ShaderHandle &
    {
        return default_vertex_shader_handle_;
    }

    auto default_vertex_shader() const -> ShaderHandle
    {
        return default_vertex_shader_handle_;
    }

    auto default_fragment_shader() -> ShaderHandle &
    {
        return default_fragment_shader_handle_;
    }

    auto default_fragment_shader() const -> ShaderHandle
    {
        return default_fragment_shader_handle_;
    }

  private:
    std::unordered_map<ShaderHandle, Shader> shaders_;
    std::unordered_map<MeshHandle, Mesh> meshes_;
    std::unordered_map<Texture2DHandle, Texture2D> textures_;
    std::unordered_map<MaterialHandle, Material> materials_;
    ShaderHandle default_vertex_shader_handle_;   // temporary - eventually we won't need these
    ShaderHandle default_fragment_shader_handle_; // temporary - eventually we won't need these

    template <typename MapType>
    auto get_map_(this auto &&self) -> auto &&
    {
        if constexpr (std::is_same_v<MapType, Shader>)
        {
            return self.shaders_;
        }
        else if constexpr (std::is_same_v<MapType, Mesh>)
        {
            return self.meshes_;
        }
        else if constexpr (std::is_same_v<MapType, Texture2D>)
        {
            return self.textures_;
        }
        else if constexpr (std::is_same_v<MapType, Material>)
        {
            return self.materials_;
        }
        else
        {
            static_assert(sizeof(MapType) == 0, "unsupported resource type");
            std::unreachable();
        }
    }
};

} // namespace pong
