#include "resource_loader.h"

#include <cstdint>
#include <filesystem>
#include <format>
#include <string_view>
#include <utility>
#include <vector>

#include <spirv-tools/libspirv.h>

#include "stb_image/stb_image.h"

#include "core/entity.h"
#include "core/resource_handles.h"
#include "core/scene.h"
#include "engine/engine_error.h"
#include "engine/file.h"
#include "engine/resource_manager.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_pipeline_manager.h"
#include "gltf/fastgltf_wrapper.h"
#include "graphics/image.h"
#include "graphics/shader.h"
#include "graphics/texture2d.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{
using namespace std::literals;

ResourceLoader::ResourceLoader(
    const VulkanDevice &device,
    ResourceManager &resource_manager,
    VulkanPipelineManager &pipeline_manager,
    std::filesystem::path absolute_path_to_assets)
    : device_{device}
    , resource_manager_{resource_manager}
    , pipeline_manager_{pipeline_manager}
    , absolute_path_to_assets_{absolute_path_to_assets}
    , command_context_{device_, "resource_loader_command_context"sv}
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
    arm::log::debug("gltf loaded default scene index/name: {}: {}", default_scene_index, default_scene.name);

    auto entities = std::vector<Entity>();
    auto root_indices = std::vector<EntityIndex>();
    for (auto i = 0zu; i < default_scene.root_node_indices.size(); ++i)
    {
        const auto &node = loaded_asset.nodes[default_scene.root_node_indices[i]];
        root_indices.push_back(process_loaded_node_(loaded_asset, node, entities));
    }
    return {std::move(entities), std::move(root_indices)};
}

auto ResourceLoader::load(std::string_view name, std::filesystem::path path) -> ShaderHandle
{
    auto file = File(path);
    auto bytes = file.data().size_bytes();
    arm::ensure(bytes % sizeof(std::uint32_t) == 0, "shader data is not 4-byte aligned");

    auto words = std::vector<std::uint32_t>();
    words.resize(bytes / sizeof(std::uint32_t));
    std::memcpy(words.data(), file.data().data(), bytes);
    arm::ensure(spirv_validate_(words), "invalid shader {}", name);

    return resource_manager_.insert<Shader>(name, {device_, name, words});
}

auto ResourceLoader::load(std::string_view name, Image &image) -> Texture2DHandle
{
    auto texture = Texture2D{image, device_};
    texture.upload_pixels(command_context_, image);

    return resource_manager_.insert<Texture2D>(name, std::move(texture));
}

auto ResourceLoader::set_fallback_texture(Texture2DHandle handle) -> void
{
    if (handle == Texture2DHandle{INVALID_RESOURCE_ID})
    {
        throw arm::Exception("invalid handle for fallback texture");
    }
    if (fallback_texture_handle_ != Texture2DHandle{INVALID_RESOURCE_ID})
    {
        arm::log::error("tried to reset resource loader's fallback texture, ignoring");
    }
    else
    {
        fallback_texture_handle_ = handle;
    }
}

auto ResourceLoader::get_or_fallback_(std::optional<Texture2DHandle> texture_handle) -> Texture2D &
{
    if (texture_handle.has_value())
    {
        if (resource_manager_.contains<Texture2D>(texture_handle.value()))
        {
            return resource_manager_.get<Texture2D>(texture_handle.value());
        }
    }
    return resource_manager_.get<Texture2D>(fallback_texture_handle_);
}

auto ResourceLoader::upload_texture_(const LoadedAsset &asset, std::size_t texture_index, ImageFormat format)
    -> Texture2DHandle
{
    auto &loaded_texture = asset.textures[texture_index];
    auto &loaded_image = asset.images[loaded_texture.image_index];
    auto &data = loaded_image.data;
    auto width = int{};
    auto height = int{};
    auto num_channels = int{};
    auto pixels = stbi_load_from_memory(
        reinterpret_cast<const stbi_uc *>(data.data()),
        static_cast<int>(data.size()),
        &width,
        &height,
        &num_channels,
        STBI_rgb_alpha);
    auto extent = Extent2D{static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)};
    auto image =
        Image{loaded_image.name, extent, format, {pixels, static_cast<std::size_t>(width) * height * STBI_rgb_alpha}};

    auto texture = Texture2D(image, device_);
    texture.upload_pixels(command_context_, image);

    auto result = resource_manager_.insert<Texture2D>(loaded_image.name, std::move(texture));
    stbi_image_free(pixels);

    return result;
}

auto ResourceLoader::process_loaded_node_(
    const LoadedAsset &loaded_asset,
    const LoadedNode &node,
    std::vector<Entity> &entities) -> EntityIndex
{
    auto entity = Entity{};
    entity.set_transform(node.local_transform);
    entity.set_name(node.name);
    arm::log::debug("  gltf node: {}", node.name);

    if (node.mesh_index.has_value())
    {
        auto mesh_index = node.mesh_index.value();
        auto model = Model{};
        model.name = loaded_asset.meshes[mesh_index].name;

        const auto &primitives = loaded_asset.meshes[mesh_index].primitives;
        for (auto i = 0zu; i < primitives.size(); ++i)
        {
            auto name = std::format("{}_{}", model.name, i);
            auto mesh_handle =
                resource_manager_.insert<Mesh>(name, {device_, name, primitives[i].vertices, primitives[i].indices});
            arm::log::debug("    added mesh {} {:x}", name, mesh_handle.value);

            auto renderable = Renderable{};
            if (primitives[i].material_index.has_value())
            {
                auto &loaded_material = loaded_asset.materials[primitives[i].material_index.value()];
                auto material = Material{
                    device_,
                    loaded_material.name,
                    pipeline_manager_.allocate_material_descriptor_set(),
                    loaded_material.base_color_factor,
                    loaded_material.metallic_factor,
                    loaded_material.roughness_factor,
                    loaded_material.alpha_mode};
                if (loaded_material.base_color_texture_index.has_value())
                {
                    material.set_base_color_texture_handle(upload_texture_(
                        loaded_asset, loaded_material.base_color_texture_index.value(), ImageFormat::RGBA8));
                }
                if (loaded_material.metallic_roughness_texture_index.has_value())
                {
                    material.set_metallic_roughness_texture_handle(upload_texture_(
                        loaded_asset, loaded_material.metallic_roughness_texture_index.value(), ImageFormat::RGBA8));
                }
                if (loaded_material.normal_texture.has_value())
                {
                    material.set_normal_texture_handle(
                        upload_texture_(loaded_asset, loaded_material.normal_texture.value(), ImageFormat::RGBA8));
                }

                auto &base_texture = get_or_fallback_(material.get_base_color_texture_handle());

                auto base_descriptor_image_info = ::vk::DescriptorImageInfo{};
                base_descriptor_image_info.sampler = base_texture.sampler();
                base_descriptor_image_info.imageView = base_texture.image_view();
                base_descriptor_image_info.imageLayout = ::vk::ImageLayout::eShaderReadOnlyOptimal;

                auto base_write_descriptor_set = ::vk::WriteDescriptorSet{};
                base_write_descriptor_set.sType = ::vk::StructureType::eWriteDescriptorSet;
                base_write_descriptor_set.pNext = nullptr;
                base_write_descriptor_set.dstSet = material.descriptor_set();
                base_write_descriptor_set.dstBinding = 0;
                base_write_descriptor_set.dstArrayElement = 0;
                base_write_descriptor_set.descriptorCount = 1;
                base_write_descriptor_set.descriptorType = ::vk::DescriptorType::eCombinedImageSampler;
                base_write_descriptor_set.pImageInfo = &base_descriptor_image_info;
                base_write_descriptor_set.pBufferInfo = nullptr;
                base_write_descriptor_set.pTexelBufferView = nullptr;

                auto &metal_texture = get_or_fallback_(material.get_metallic_roughness_texture_handle());

                auto metal_descriptor_image_info = ::vk::DescriptorImageInfo{};
                metal_descriptor_image_info.sampler = metal_texture.sampler();
                metal_descriptor_image_info.imageView = metal_texture.image_view();
                metal_descriptor_image_info.imageLayout = ::vk::ImageLayout::eShaderReadOnlyOptimal;

                auto metal_write_descriptor_set = ::vk::WriteDescriptorSet{};
                metal_write_descriptor_set.sType = ::vk::StructureType::eWriteDescriptorSet;
                metal_write_descriptor_set.pNext = nullptr;
                metal_write_descriptor_set.dstSet = material.descriptor_set();
                metal_write_descriptor_set.dstBinding = 1;
                metal_write_descriptor_set.dstArrayElement = 0;
                metal_write_descriptor_set.descriptorCount = 1;
                metal_write_descriptor_set.descriptorType = ::vk::DescriptorType::eCombinedImageSampler;
                metal_write_descriptor_set.pImageInfo = &metal_descriptor_image_info;
                metal_write_descriptor_set.pBufferInfo = nullptr;
                metal_write_descriptor_set.pTexelBufferView = nullptr;

                auto &normal_texture = get_or_fallback_(material.get_normal_texture_handle());

                auto normal_descriptor_image_info = ::vk::DescriptorImageInfo{};
                normal_descriptor_image_info.sampler = normal_texture.sampler();
                normal_descriptor_image_info.imageView = normal_texture.image_view();
                normal_descriptor_image_info.imageLayout = ::vk::ImageLayout::eShaderReadOnlyOptimal;

                auto normal_write_descriptor_set = ::vk::WriteDescriptorSet{};
                normal_write_descriptor_set.sType = ::vk::StructureType::eWriteDescriptorSet;
                normal_write_descriptor_set.pNext = nullptr;
                normal_write_descriptor_set.dstSet = material.descriptor_set();
                normal_write_descriptor_set.dstBinding = 2;
                normal_write_descriptor_set.dstArrayElement = 0;
                normal_write_descriptor_set.descriptorCount = 1;
                normal_write_descriptor_set.descriptorType = ::vk::DescriptorType::eCombinedImageSampler;
                normal_write_descriptor_set.pImageInfo = &normal_descriptor_image_info;
                normal_write_descriptor_set.pBufferInfo = nullptr;
                normal_write_descriptor_set.pTexelBufferView = nullptr;

                auto material_descriptor_buffer_info = ::vk::DescriptorBufferInfo{};
                material_descriptor_buffer_info.buffer = material.get_buffer().native_handle();
                material_descriptor_buffer_info.offset = 0;
                material_descriptor_buffer_info.range = sizeof(UBO_Material);

                auto material_write_descriptor_set = ::vk::WriteDescriptorSet{};
                material_write_descriptor_set.sType = ::vk::StructureType::eWriteDescriptorSet;
                material_write_descriptor_set.pNext = nullptr;
                material_write_descriptor_set.dstSet = material.descriptor_set();
                material_write_descriptor_set.dstBinding = 3;
                material_write_descriptor_set.dstArrayElement = 0;
                material_write_descriptor_set.descriptorCount = 1;
                material_write_descriptor_set.descriptorType = ::vk::DescriptorType::eUniformBuffer;
                material_write_descriptor_set.pImageInfo = nullptr;
                material_write_descriptor_set.pBufferInfo = &material_descriptor_buffer_info;
                material_write_descriptor_set.pTexelBufferView = nullptr;

                device_.native_handle().updateDescriptorSets(
                    {base_write_descriptor_set,
                     metal_write_descriptor_set,
                     normal_write_descriptor_set,
                     material_write_descriptor_set},
                    {});

                auto material_handle = resource_manager_.insert<Material>(material.name(), std::move(material));
                renderable = Renderable{mesh_handle, std::make_optional(material_handle)};
                arm::log::debug("    added material {} {:x}", loaded_material.name, material_handle.value);
            }
            else
            {
                renderable = Renderable{mesh_handle, std::nullopt};
            }
            model.renderables.push_back(std::move(renderable));
        }
        entity.model() = std::make_optional(std::move(model));
    }
    else
    {
        // mesh_index does not have a value. could be transform-only, camera, light, skin
        // do stuff with those here. for now, we'll just explicitly state that there's no model for this entity,
        // even tho it default constructs that way
        entity.model() = std::nullopt;
    }

    for (auto i = 0zu; i < node.child_indices.size(); ++i)
    {
        entity.add_child(process_loaded_node_(loaded_asset, loaded_asset.nodes[node.child_indices[i]], entities));
    }

    arm::log::debug("Creating entity '{}'", entity.name());
    auto result = EntityIndex{entities.size()};
    entities.push_back(std::move(entity));
    return result;
}

auto ResourceLoader::spirv_validate_(std::span<const std::uint32_t> words) -> bool
{
    if (words.size() < 5)
        return false; // automatically not valid;

    const auto context = spvContextCreate(spv_target_env::SPV_ENV_VULKAN_1_3);
    auto diagnostic = spv_diagnostic{};
    auto binary = spv_const_binary_t{.code = words.data(), .wordCount = words.size()};
    auto validate_result = spvValidate(context, &binary, &diagnostic);
    if (validate_result)
    {
        if (diagnostic)
        {
            arm::log::info("message from spirv-tools spvValidate: {}", diagnostic->error);
            spvDiagnosticDestroy(diagnostic);
        }
    }
    spvContextDestroy(context);
    return validate_result == SPV_SUCCESS;
}

} // namespace pong
