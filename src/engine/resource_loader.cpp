#include "resource_loader.h"

#include <cstdint>
#include <filesystem>
#include <string_view>
#include <vector>

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

auto ResourceLoader::load(std::string_view name, std::filesystem::path path) -> MeshHandle
{
    const auto key = MeshHandle{get_resource_id_(name)};

    if (resource_manager_.contains<Mesh>(key))
    {
        arm::log::warn("mesh already loaded: {} (skipping)", name);
        return key;
    }

    auto gltf = FastGLTFWrapper();
    auto loaded_asset = gltf.load(path);

    return resource_manager_.insert<Mesh>(
        name,
        Mesh{
            device_,
            loaded_asset.meshes[0].name,
            loaded_asset.meshes[0].primitives[0].vertices,
            loaded_asset.meshes[0].primitives[0].indices});
}

auto ResourceLoader::load(std::string_view name, Image &image) -> Texture2DHandle
{
    auto texture = Texture2D{image, device_};
    texture.upload_pixels(command_context_, image);

    return resource_manager_.insert<Texture2D>(name, std::move(texture));
}

auto ResourceLoader::get_resource_id_(std::string_view name) -> std::uint64_t
{
    return hash_string(name);
}

} // namespace pong
