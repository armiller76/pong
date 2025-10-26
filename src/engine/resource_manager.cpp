#include "resource_manager.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "engine/file.h"
#include "engine/vulkan/vulkan_device.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

ResourceManager::ResourceManager(const VulkanDevice &device)
    : device_(device)
    , shaders_({})
    , meshes_({})
{
    arm::log::debug("ResourceManager constructor");
}

auto ResourceManager::load(std::string_view name, const std::filesystem::path &path, ::vk::ShaderStageFlagBits stage)
    -> Shader &
{
    if (auto entry = shaders_.find(name); entry != shaders_.end())
    {
        arm::log::warn("shader already loaded: {} (skipping)", name);
        return entry->second;
    }

    auto spirv = File::read_shader(path).as_spirv();
    auto key = std::string{name};

    auto [entry, inserted] = shaders_.try_emplace(
        std::move(key), std::string(name), std::vector<std::uint32_t>(spirv.begin(), spirv.end()), stage);
    arm::ensure(inserted, "failed to load shader: {}", name);

    return entry->second;
}

auto ResourceManager::load(std::string_view name, std::span<const Vertex> vertices) -> Mesh &
{
    if (auto entry = meshes_.find(name); entry != meshes_.end())
    {
        arm::log::warn("mesh already loaded: {} (skipping)", name);
        return entry->second;
    }

    auto key = std::string{name};
    auto [entry, inserted] = meshes_.try_emplace(std::move(key), device_, vertices);
    arm::ensure(inserted, "failed to load mesh: {}", name);

    return entry->second;
}

}
