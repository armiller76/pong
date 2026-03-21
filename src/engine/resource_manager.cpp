#include "resource_manager.h"

#include <filesystem>
#include <string>

#include "engine/file.h"
#include "engine/vulkan/vulkan_device.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/hash.h"
#include "utils/log.h"

namespace pong
{

ResourceManager::ResourceManager(const VulkanDevice &device)
    : device_(device)
{
    arm::log::debug("ResourceManager constructor");
}

auto ResourceManager::load(std::string name, const std::filesystem::path &path, ShaderStage stage) -> Shader &
{
    const auto key = get_resource_id_(name);

    if (auto entry = shaders_.find(key); entry != shaders_.end())
    {
        arm::log::warn("shader already loaded: {} (skipping)", name);
        return entry->second;
    }

    auto [entry, inserted] = shaders_.try_emplace(key);
    arm::ensure(inserted, "failed to load shader: {}", name);

    entry->second.name = std::move(name);
    entry->second.stage = stage;
    entry->second.spirv.assign_range(File(path).data());

    return entry->second;
}

auto ResourceManager::load(Mesh mesh) -> Mesh &
{
    const auto key = get_resource_id_(mesh.name());

    if (auto entry = meshes_.find(key); entry != meshes_.end())
    {
        arm::log::warn("mesh already loaded: {} (skipping)", mesh.name());
        return entry->second;
    }

    auto [entry, inserted] = meshes_.try_emplace(key, std::move(mesh));
    arm::ensure(inserted, "failed to load mesh: {}", mesh.name());

    return entry->second;
}

constexpr auto ResourceManager::get_resource_id_(std::string_view str) -> std::uint64_t
{
    return pong::hash_string(str);
}

}
