#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <string_view>
#include <vector>

#include "core/resource_handles.h"
#include "core/scene.h"
#include "engine/engine_error.h"
#include "engine/vulkan/vulkan_immediate_command_context.h"
#include "gltf/fastgltf_primitives.h"
#include "graphics/image_format.h"

namespace pong
{

class Entity;
class Image;
class VulkanPipelineManager;
class ResourceManager;
class Texture2D;
class VulkanDevice;
enum class ShaderStage;

class ResourceLoader
{
  public:
    ResourceLoader(
        const VulkanDevice &device,
        ResourceManager &resource_manager,
        VulkanPipelineManager &pipeline_manager,
        std::filesystem::path absolute_path_to_assets);

    auto loadgltf(std::filesystem::path path) -> Scene;

    // Shader loading
    auto load(std::string_view name, std::filesystem::path path) -> ShaderHandle;
    // Texture loading
    auto load(std::string_view name, Image &image) -> Texture2DHandle;

    auto set_fallback_texture(Texture2DHandle handle) -> void;

  private:
    const VulkanDevice &device_;
    ResourceManager &resource_manager_;
    VulkanPipelineManager &pipeline_manager_;
    std::filesystem::path absolute_path_to_assets_;
    VulkanImmediateCommandContext command_context_;
    Texture2DHandle fallback_texture_handle_ = Texture2DHandle{INVALID_RESOURCE_ID};

    auto get_or_fallback_(std::optional<Texture2DHandle> texture_handle) -> Texture2D &;
    auto upload_texture_(const LoadedAsset &asset, std::size_t texture_index, ImageFormat format) -> Texture2DHandle;
    auto process_loaded_node_(LoadedAsset &asset, const LoadedNode &node, std::vector<Entity> &entities) -> EntityIndex;
    auto spirv_validate_(std::span<const std::uint32_t> words) -> bool;
}; // class ResourceLoader

} // namespace pong
