#include "resource_loader.h"

#include <cstdint>
#include <filesystem>
#include <format>
#include <string_view>
#include <vector>

#include "core/entity.h"
#include "core/scene.h"
#include "engine/file.h"
#include "engine/vulkan/vulkan_device.h"
#include "graphics/shader.h"
#include "graphics/utils.h"
#include "resource_manager.h"
#include "utils/hash.h"
#include "utils/log.h"

namespace pong
{
using namespace std::literals;

ResourceLoader::ResourceLoader(
    const VulkanDevice &device,
    ResourceManager &resource_manager,
    std::filesystem::path absolute_path_to_assets)
    : device_{device}
    , command_context_{device_, "resource_loader_command_context"sv}
    , absolute_path_to_assets_{absolute_path_to_assets}
    , resource_manager_{resource_manager}
{
    arm::log::debug("ResourceLoader constructor");
}

auto ResourceLoader::loadgltf(std::filesystem::path path) -> Scene
{
    auto gltf = FastGLTFWrapper();
    const auto loaded_asset = gltf.load(path);

    // const auto scene_count = loaded_asset.scenes.size();
    const auto default_scene_index = loaded_asset.default_scene_index.value_or(0);
    const auto &default_scene = loaded_asset.scenes[default_scene_index];

    arm::log::debug(
        "gltf loaded:\n default scene index: {}\ndefault scene name: {}", default_scene_index, default_scene.name);
    auto entities = std::vector<Entity>();
    auto root_indices = std::vector<EntityIndex>();
    for (auto i = 0zu; i < default_scene.root_node_indices.size(); ++i)
    {
        const auto &node = loaded_asset.nodes[default_scene.root_node_indices[i]];
        arm::log::debug("processing node {} with index #{}", node.name, default_scene.root_node_indices[i]);
        root_indices.push_back(process_loaded_node_(loaded_asset, node, entities));
    }
    return {std::move(entities), std::move(root_indices)};
}

auto ResourceLoader::load(std::string_view name, std::filesystem::path path, ShaderStage stage) -> ShaderHandle
{
    auto file = File(path);
    auto bytes = file.data().size_bytes();
    arm::ensure(bytes % sizeof(std::uint32_t) == 0, "shader data is not 4-byte aligned");

    auto words = std::vector<std::uint32_t>();
    words.resize(bytes / sizeof(std::uint32_t));
    std::memcpy(words.data(), file.data().data(), bytes);
    arm::ensure(spirv_validate(words), "invalid shader {}", name);

    auto shader = Shader{};
    shader.name = name;
    shader.stage = stage;
    shader.spirv.assign_range(words);

    return resource_manager_.insert<Shader>(name, std::move(shader));
}

auto ResourceLoader::load(std::string_view name, Image &image) -> Texture2DHandle
{
    auto texture = Texture2D{image, device_};
    texture.upload_pixels(command_context_, image);

    return resource_manager_.insert<Texture2D>(name, std::move(texture));
}

auto ResourceLoader::process_loaded_node_(
    const LoadedAsset &asset,
    const LoadedNode &node,
    std::vector<Entity> &entities) -> EntityIndex
{
    auto entity = Entity{};
    entity.set_transform(node.local_transform);
    entity.set_name(node.name);

    if (node.mesh_index.has_value())
    {
        // if mesh_index has a value, this is effectively an Entity.
        // Get the transform from node, model from its mesh
        arm::log::debug("  - node has mesh_index");
        auto mesh_index = node.mesh_index.value();
        auto model = Model{};
        model.name = asset.meshes[mesh_index].name;

        const auto &primitives = asset.meshes[mesh_index].primitives;
        arm::log::debug("  - node has {} primitives", primitives.size());

        for (auto i = 0zu; i < primitives.size(); ++i)
        {
            auto name = std::format("{}_{}", model.name, i);
            auto mesh_handle =
                resource_manager_.insert<Mesh>(name, {device_, name, primitives[i].vertices, primitives[i].indices});
            arm::log::debug("    - added mesh {} {}", name, mesh_handle.value);

            auto renderable = Renderable{};
            if (primitives[i].material_index.has_value())
            {
                auto material_name = asset.materials[primitives[i].material_index.value()].name;
                auto material_handle = resource_manager_.insert<Material>(material_name, {material_name});
                renderable = Renderable{mesh_handle, std::make_optional(material_handle)};
                arm::log::debug("    - with material {} {}", material_name, material_handle.value);
            }
            else
            {
                renderable = Renderable{mesh_handle, std::nullopt};
                arm::log::debug("    - without material");
            }
            model.renderables.push_back(std::move(renderable));
        }
        entity.model() = std::make_optional(std::move(model));
    }
    else
    {
        // mesh_index does not have a value. could be transform-only, camera, light, skin
        // do stuff with those here. for now, we'll just explicitly state that there's no model for this entity, even
        // tho it default constructs that way
        entity.model() = std::nullopt;
    }

    for (auto i = 0zu; i < node.child_indices.size(); ++i)
    {
        arm::log::debug("Moving to child node, index #{}", node.child_indices[i]);
        entity.add_child(process_loaded_node_(asset, asset.nodes[node.child_indices[i]], entities));
    }

    arm::log::debug("Creating entity \"{}\"", entity.name());
    auto result = EntityIndex{entities.size()};
    entities.push_back(std::move(entity));
    return result;
}

auto ResourceLoader::get_resource_id_(std::string_view name) -> std::uint64_t
{
    return hash_string(name);
}

} // namespace pong
