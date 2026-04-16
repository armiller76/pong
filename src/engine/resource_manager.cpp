#include "resource_manager.h"

#include <cstring>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "engine/file.h"
#include "engine/vulkan/vulkan_device.h"
#include "gltf/fastgltf_wrapper.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "graphics/utils.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/hash.h"
#include "utils/log.h"

namespace pong
{

auto load_mesh_from_file(const VulkanDevice &device, std::filesystem::path path) -> Mesh;

ResourceManager::ResourceManager(const VulkanDevice &device)
    : device_{device}
    , shaders_{}
    , meshes_{}
    , textures_{}
    , command_context_{device_, "resource_manager"}
{
    arm::log::debug("ResourceManager constructor");
}

auto ResourceManager::load(std::string_view name, const std::filesystem::path &path, ShaderStage stage) -> ShaderHandle
{
    auto file = File(path);
    auto bytes = file.data().size_bytes();
    arm::ensure(bytes % sizeof(std::uint32_t) == 0, "shader data is not 4-byte aligned");

    auto words = std::vector<std::uint32_t>();
    words.resize(bytes / sizeof(std::uint32_t));
    std::memcpy(words.data(), file.data().data(), bytes);

    if (!spirv_validate(words))
    {
        throw arm::Exception("invalid shader {}", name);
    }

    const auto key = ShaderHandle{get_resource_id_(name)};

    auto [entry, inserted] = shaders_.try_emplace(key);
    if (!inserted)
    {
        arm::log::warn("shader already loaded: {} (skipping)", name);
        return key;
    }

    entry->second.name = name;
    entry->second.stage = stage;
    entry->second.spirv.assign_range(words);

    return key;
}

auto ResourceManager::load(Mesh &&mesh) -> MeshHandle
{
    auto key = MeshHandle{get_resource_id_(mesh.name())};

    auto [entry, inserted] = meshes_.try_emplace(key, std::move(mesh));
    if (!inserted)
    {
        arm::log::warn("mesh already loaded: {} (skipping)", mesh.name());
    }

    return key;
}

auto ResourceManager::load(std::string_view name, const std::filesystem::path &path) -> MeshHandle
{
    const auto key = MeshHandle{get_resource_id_(name)};

    if (get_map_<Mesh>().contains(key))
    {
        arm::log::warn("mesh already loaded: {} (skipping)", name);
        return key;
    }

    auto gltf = FastGLTFWrapper();
    auto loaded_asset = gltf.load(path);

    auto [entry, inserted] = meshes_.try_emplace(
        key,
        Mesh{
            loaded_asset.meshes[0].name,
            device_,
            loaded_asset.meshes[0].primitives[0].vertices,
            loaded_asset.meshes[0].primitives[0].indices});
    if (!inserted)
    {
        throw arm::Exception("error converting gltf primitive -> Mesh");
    }

    return key;
}

auto ResourceManager::load(Texture2D &&texture) -> Texture2DHandle
{
    const auto key = Texture2DHandle{get_resource_id_(texture.name())};

    auto [entry, inserted] = textures_.try_emplace(key, std::move(texture));
    if (!inserted)
    {
        arm::log::warn("texture already loaded: {} (skipping)", entry->second.name());
    }
    return key;
}

auto ResourceManager::load(std::string_view name, Image &image) -> Texture2DHandle
{
    const auto key = Texture2DHandle{get_resource_id_(name)};

    auto [entry, inserted] = textures_.try_emplace(key, std::move(Texture2D{image, device_}));
    if (!inserted)
    {
        arm::log::warn("texture already loaded: {} (skipping)", entry->second.name());
    }

    auto &texture = entry->second;

    texture.upload_pixels(command_context_, image);
    return key;
}

auto load_mesh_from_file(const VulkanDevice &device, std::filesystem::path path) -> Mesh
{
    auto gltf = FastGLTFWrapper();
    auto loaded_asset = gltf.load(path);

    return Mesh{
        loaded_asset.meshes[0].name,
        device,
        loaded_asset.meshes[0].primitives[0].vertices,
        loaded_asset.meshes[0].primitives[0].indices};
}

}
