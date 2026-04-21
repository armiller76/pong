#pragma once

#include <filesystem>
#include <string_view>
#include <vector>

#include "core/resource_handles.h"
#include "core/scene.h"
#include "engine/vulkan/vulkan_immediate_command_context.h"
#include "gltf/fastgltf_primitives.h"
#include "graphics/image_format.h"
#include "resource_manager.h"

namespace pong
{

class Entity;
class Image;
class VulkanDevice;
enum class ShaderStage;

class ResourceLoader
{
  public:
    ResourceLoader(
        const VulkanDevice &device,
        ResourceManager &resource_manager,
        std::filesystem::path absolute_path_to_assets);

    auto loadgltf(std::filesystem::path path) -> Scene;

    // Shader loading
    auto load(std::string_view name, std::filesystem::path path, ShaderStage stage) -> ShaderHandle;
    // Texture loading
    auto load(std::string_view name, Image &image) -> Texture2DHandle;

  private:
    const VulkanDevice &device_;
    ResourceManager &resource_manager_;
    std::filesystem::path absolute_path_to_assets_;
    VulkanImmediateCommandContext command_context_;
    std::optional<Texture2DHandle> fallback_texture_handle_;

    auto get_or_fallback_(std::optional<Texture2DHandle> texture_handle) -> Texture2D &;
    auto upload_texture_(const LoadedAsset &asset, std::size_t texture_index, ImageFormat format) -> Texture2DHandle;
    auto process_loaded_node_(const LoadedAsset &asset, const LoadedNode &node, std::vector<Entity> &entities)
        -> EntityIndex;
}; // class ResourceLoader

} // namespace pong
