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

auto ResourceManager::load(std::string name, const std::filesystem::path &path, ShaderStage stage) -> std::uint64_t
{
    const auto key = get_resource_id(name);

    if (auto entry = shaders_.find(key); entry != shaders_.end())
    {
        arm::log::warn("shader already loaded: {} (skipping)", name);
        return key;
    }

    auto [entry, inserted] = shaders_.try_emplace(key);
    arm::ensure(inserted, "failed to load shader: {}", name);

    entry->second.name = std::move(name);
    entry->second.stage = stage;
    entry->second.spirv.assign_range(File(path).as_spirv());

    return key;
}

auto ResourceManager::load(Mesh &&mesh) -> std::uint64_t
{
    const auto key = get_resource_id(mesh.name());

    auto [it, inserted] = meshes_.try_emplace(key, std::move(mesh));
    if (!inserted)
    {
        arm::log::warn("mesh already loaded: {} (skipping)", it->second.name());
    }
    return key;
}

}
