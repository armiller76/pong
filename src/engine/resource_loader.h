#pragma once

#include <cstdint>
#include <filesystem>
#include <string_view>
#include <vector>

#include "core/resource_handles.h"
#include "engine/vulkan/vulkan_immediate_command_context.h"
#include "gltf/fastgltf_primitives.h"
#include "gltf/fastgltf_wrapper.h"
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

    auto loadgltf(std::filesystem::path path) -> std::vector<Entity>;

    // Shader loading
    auto load(std::string_view name, std::filesystem::path path, ShaderStage stage) -> ShaderHandle;
    // Texture loading
    auto load(std::string_view name, Image &image) -> Texture2DHandle;

  private:
    const VulkanDevice &device_;
    VulkanImmediateCommandContext command_context_;
    std::filesystem::path absolute_path_to_assets_;
    ResourceManager &resource_manager_;

    auto process_loaded_node_(
        const LoadedAsset &asset,
        const LoadedNode &node,
        std::vector<Entity> &entities,
        const ::glm::mat4x4 &parent_transform) -> void;
    auto get_resource_id_(std::string_view name) -> std::uint64_t;
}; // class ResourceLoader

} // namespace pong
