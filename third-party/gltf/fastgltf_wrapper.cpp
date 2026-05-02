#include "fastgltf_wrapper.h"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string_view>
#include <vector>

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

#include "fastgltf/types.hpp"
#include "fastgltf_glm_utils.h"
#include "fastgltf_primitives.h"
#include "graphics/types.h"
#include "graphics/vertex.h"
#include "utils/error.h"

namespace
{
// helper lambda to look up accessor index by attribute name
auto query_attribute(const ::fastgltf::Primitive &primitive, std::string_view attribute_name)
    -> std::optional<std::size_t>
{
    auto entry = std::ranges::find(primitive.attributes, attribute_name, &::fastgltf::Attribute::name);
    if (entry == primitive.attributes.end())
    {
        return std::nullopt;
    }
    return entry->accessorIndex;
}

template <typename T, typename Setter>
auto fill_attribute(
    ::fastgltf::Asset &asset,
    const ::fastgltf::Primitive &primitive,
    std::string_view attribute_name,
    std::vector<pong::Vertex> &vertices,
    const T &default_value,
    Setter setter) -> void
{
    auto accessor_index = query_attribute(primitive, attribute_name);
    if (accessor_index.has_value())
    {
        ::fastgltf::iterateAccessorWithIndex<T>(
            asset,
            asset.accessors[accessor_index.value()],
            [&](const T &value, std::size_t i) { setter(vertices[i], value); });
    }
    else
    {
        for (std::size_t i = 0; i < vertices.size(); ++i)
        {
            setter(vertices[i], default_value);
        }
    }
}

} // anonymous namespace

namespace pong
{

FastGLTFWrapper::FastGLTFWrapper()
    : parser_{std::make_unique<::fastgltf::Parser>(
          ::fastgltf::Extensions::KHR_mesh_quantization | ::fastgltf::Extensions::KHR_texture_transform
          | ::fastgltf::Extensions::KHR_materials_unlit)}
{
    arm::log::debug("FastGLTFWrapper constructor");
}

FastGLTFWrapper::~FastGLTFWrapper() = default;

auto FastGLTFWrapper::load(std::filesystem::path path) -> LoadedAsset
{
    // create the FastGltf buffer from the raw Gltf file
    auto data = ::fastgltf::GltfDataBuffer::FromPath(path);
    arm::ensure(
        data.error() == ::fastgltf::Error::None,
        "FastGLTF: unable to load {} ({})",
        path.filename().string(),
        ::fastgltf::getErrorMessage(data.error()));

    // use the FastGltf parser to create the FastGltf "asset"
    auto gltf_asset = parser_->loadGltf(
        data.get(),
        path.parent_path(),
        ::fastgltf::Options::LoadGLBBuffers | ::fastgltf::Options::LoadExternalBuffers
            | ::fastgltf::Options::GenerateMeshIndices);
    // check for error from FastGltf and fail loudly on error
    arm::ensure(
        gltf_asset.error() == ::fastgltf::Error::None,
        "FastGLTF: unable to load {} ({})",
        path.filename().string(),
        ::fastgltf::getErrorMessage(gltf_asset.error()));

    // if we made it this far, we have parsed the gltf data into 'gltf_asset'

    // for now we'll only load ONE scene - if there's a default, use that, otherwise fall back to index 0
    // in the future, if needed, we can implement multiple scenes
    auto loaded_asset = LoadedAsset{};
    loaded_asset.default_scene_index =
        gltf_asset->defaultScene.has_value() ? std::make_optional(gltf_asset->defaultScene.value()) : std::nullopt;

    extract_scenes_(gltf_asset.get(), loaded_asset);
    extract_nodes_(gltf_asset.get(), loaded_asset);
    extract_materials_(gltf_asset.get(), loaded_asset);
    extract_meshes_(gltf_asset.get(), loaded_asset);
    extract_textures_(gltf_asset.get(), loaded_asset);
    extract_samplers_(gltf_asset.get(), loaded_asset);

    // note: image extraction assumes .GLB format! no external URIs just yet
    extract_images_(gltf_asset.get(), loaded_asset);

    return loaded_asset;
}

auto FastGLTFWrapper::extract_scenes_(::fastgltf::Asset &asset, LoadedAsset &loaded_asset) -> void
{
    for (const auto &scene : asset.scenes)
    {
        auto loaded_scene = LoadedScene{};
        loaded_scene.name = scene.name;
        loaded_scene.root_node_indices.assign_range(scene.nodeIndices);
        loaded_asset.scenes.push_back(std::move(loaded_scene));
    }
}

auto FastGLTFWrapper::extract_nodes_(::fastgltf::Asset &asset, LoadedAsset &loaded_asset) -> void
{
    if (asset.nodes.empty())
    {
        return;
    }
    else
    {
        for (const auto &node : asset.nodes)
        {
            auto loaded_node = LoadedNode{};
            loaded_node.name = node.name;
            loaded_node.mesh_index =
                node.meshIndex.has_value() ? std::make_optional(node.meshIndex.value()) : std::nullopt;
            if (auto *trs = std::get_if<::fastgltf::TRS>(&node.transform))
            {
                loaded_node.local_transform.position = to_pong(trs->translation);
                loaded_node.local_transform.rotation = to_pong(trs->rotation);
                loaded_node.local_transform.scale = to_pong(trs->scale);
            }
            else if (auto *mat = std::get_if<::fastgltf::math::fmat4x4>(&node.transform))
            {
                ::glm::vec3 skew;
                ::glm::vec4 perspective;
                ::glm::decompose(
                    to_pong(*mat),
                    loaded_node.local_transform.scale,
                    loaded_node.local_transform.rotation,
                    loaded_node.local_transform.position,
                    skew,
                    perspective);
            }
            else
            {
                throw arm::Exception("we don't know how to handle matrix transform yet");
            }
            if (node.children.size() > 0)
            {
                loaded_node.child_indices.assign_range(node.children);
            }
            loaded_asset.nodes.push_back(std::move(loaded_node));
        }
    }
}

auto FastGLTFWrapper::extract_materials_(::fastgltf::Asset &asset, LoadedAsset &loaded_asset) -> void
{
    if (asset.materials.empty())
    {
        return;
    }
    else
    {
        for (const auto &material : asset.materials)
        {
            auto loaded_material = LoadedMaterial{};
            loaded_material.name = material.name;
            loaded_material.base_color_factor = to_pong(material.pbrData.baseColorFactor);
            loaded_material.base_color_texture_index =
                material.pbrData.baseColorTexture.has_value()
                    ? std::make_optional(material.pbrData.baseColorTexture.value().textureIndex)
                    : std::nullopt;
            loaded_material.metallic_factor = material.pbrData.metallicFactor;
            loaded_material.roughness_factor = material.pbrData.roughnessFactor;
            loaded_material.metallic_roughness_texture_index =
                material.pbrData.metallicRoughnessTexture.has_value()
                    ? std::make_optional(material.pbrData.metallicRoughnessTexture.value().textureIndex)
                    : std::nullopt;
            loaded_material.normal_texture = material.normalTexture.has_value()
                                                 ? std::make_optional(material.normalTexture.value().textureIndex)
                                                 : std::nullopt;
            loaded_material.alpha_mode = to_pong(material.alphaMode);
            loaded_asset.materials.push_back(std::move(loaded_material));
        }
    }
}

auto FastGLTFWrapper::extract_meshes_(::fastgltf::Asset &data, LoadedAsset &loaded_asset) -> void
{
    if (data.meshes.empty())
    {
        return;
    }
    else
    {
        // loop thru meshes in data
        for (const auto &mesh : data.meshes)
        {
            auto loaded_mesh = LoadedMesh{};
            loaded_mesh.name = mesh.name;

            // loop thru each primitive in the mesh
            for (const auto &primitive : mesh.primitives)
            {
                auto loaded_primitive = LoadedPrimitive{};

                auto position_index = query_attribute(primitive, "POSITION");
                if (!position_index.has_value())
                {
                    // position is always required
                    throw arm::Exception("missing required position data in gltf primitive");
                }
                else
                {
                    auto position_accessor = &data.accessors[position_index.value()];
                    auto count = position_accessor->count;
                    loaded_primitive.vertices.resize(count);

                    fill_attribute<::glm::vec3>(
                        data,
                        primitive,
                        "POSITION",
                        loaded_primitive.vertices,
                        {0.0f, 0.0f, 0.0f},
                        [&](pong::Vertex &v, const ::glm::vec3 &value) { v.position = value; });

                    fill_attribute<::glm::vec3>(
                        data,
                        primitive,
                        "NORMAL",
                        loaded_primitive.vertices,
                        {0.0f, 1.0f, 0.0f},
                        [&](pong::Vertex &v, const ::glm::vec3 &value) { v.normal = value; });

                    fill_attribute<::glm::vec2>(
                        data,
                        primitive,
                        "TEXCOORD_0",
                        loaded_primitive.vertices,
                        {0.0f, 0.0f},
                        [&](pong::Vertex &v, const ::glm::vec2 &value) { v.uv = value; });

                    fill_attribute<::glm::vec4>(
                        data,
                        primitive,
                        "COLOR",
                        loaded_primitive.vertices,
                        {1.0f, 1.0f, 1.0f, 1.0f},
                        [&](pong::Vertex &v, const ::glm::vec4 &value) { v.color = value; });

                    auto indices_accessor_index = primitive.indicesAccessor;
                    arm::ensure(indices_accessor_index.has_value(), "fastgltf error: unable to find vertex indices");

                    ::fastgltf::iterateAccessor<std::uint32_t>(
                        data,
                        data.accessors[indices_accessor_index.value()],
                        [&](std::uint32_t index) { loaded_primitive.indices.push_back(index); });

                    loaded_primitive.material_index = primitive.materialIndex.has_value()
                                                          ? std::make_optional(primitive.materialIndex.value())
                                                          : std::nullopt;
                    loaded_mesh.primitives.push_back(std::move(loaded_primitive));
                }
            }
            loaded_asset.meshes.push_back(std::move(loaded_mesh));
        }
    }
}

auto FastGLTFWrapper::extract_images_(::fastgltf::Asset &asset, LoadedAsset &loaded_asset) -> void
{
    if (asset.images.empty())
    {
        return;
    }
    else
    {
        for (const auto &image : asset.images)
        {
            auto loaded_image = LoadedImage{};
            loaded_image.name = image.name;
            if (auto *buffer_view = std::get_if<::fastgltf::sources::BufferView>(&image.data))
            {
                auto &view = asset.bufferViews[buffer_view->bufferViewIndex];
                auto buffer_start =
                    std::get_if<::fastgltf::sources::Array>(&asset.buffers[view.bufferIndex].data)->bytes.data()
                    + view.byteOffset;
                loaded_image.data = {buffer_start, buffer_start + view.byteLength};
                loaded_asset.images.push_back(std::move(loaded_image));
            }
            else // if (auto *uri = std::get_if<::fastgltf::sources::URI>(&image.data))
            {
                arm::log::error("unsupported image in gltf data ({}) skipping", image.name);
                continue;
            }
            if (loaded_asset.images.empty())
            {
                arm::log::error("gltf data contained {} images, none were loaded", asset.images.size());
            }
        }
    }
}

auto FastGLTFWrapper::extract_samplers_(::fastgltf::Asset &asset, LoadedAsset &loaded_asset) -> void
{
    if (asset.samplers.empty())
    {
        return;
    }
    else
    {
        for (const auto &sampler : asset.samplers)
        {
            auto loaded_sampler = LoadedSampler{};

            if (sampler.minFilter.has_value())
            {
                switch (sampler.minFilter.value())
                {
                    case ::fastgltf::Filter::Linear:
                    {
                        loaded_sampler.min_filter = FilterMode::Linear;
                        loaded_sampler.samples_mips = false;
                        loaded_sampler.mip_map_mode = MipMapMode::Linear;
                    }
                    break;
                    case ::fastgltf::Filter::Nearest:
                    {
                        loaded_sampler.min_filter = FilterMode::Nearest;
                        loaded_sampler.samples_mips = false;
                        loaded_sampler.mip_map_mode = MipMapMode::Linear;
                    }
                    break;
                    case ::fastgltf::Filter::LinearMipMapLinear:
                    {
                        loaded_sampler.min_filter = FilterMode::Linear;
                        loaded_sampler.samples_mips = true;
                        loaded_sampler.mip_map_mode = MipMapMode::Linear;
                    }
                    break;
                    case ::fastgltf::Filter::LinearMipMapNearest:
                    {
                        loaded_sampler.min_filter = FilterMode::Linear;
                        loaded_sampler.samples_mips = true;
                        loaded_sampler.mip_map_mode = MipMapMode::Nearest;
                    }
                    break;
                    case ::fastgltf::Filter::NearestMipMapLinear:
                    {
                        loaded_sampler.min_filter = FilterMode::Nearest;
                        loaded_sampler.samples_mips = true;
                        loaded_sampler.mip_map_mode = MipMapMode::Linear;
                    }
                    break;
                    case ::fastgltf::Filter::NearestMipMapNearest:
                    {
                        loaded_sampler.min_filter = FilterMode::Nearest;
                        loaded_sampler.samples_mips = true;
                        loaded_sampler.mip_map_mode = MipMapMode::Nearest;
                    }
                    break;
                }
            }

            if (sampler.magFilter.has_value())
            {
                switch (sampler.magFilter.value())
                {
                    case ::fastgltf::Filter::Linear:
                    {
                        loaded_sampler.mag_filter = FilterMode::Linear;
                    }
                    break;
                    case ::fastgltf::Filter::Nearest:
                    {
                        loaded_sampler.mag_filter = FilterMode::Nearest;
                    }
                    break;
                }
            }

            loaded_sampler.wrap_u = to_pong(sampler.wrapS);
            loaded_sampler.wrap_v = to_pong(sampler.wrapT);
            loaded_asset.samplers.push_back(loaded_sampler);
        }
    }
}

auto FastGLTFWrapper::extract_textures_(::fastgltf::Asset &asset, LoadedAsset &loaded_asset) -> void
{
    if (asset.textures.empty())
    {
        return;
    }
    else
    {
        for (const auto &texture : asset.textures)
        {
            auto loaded_texture = LoadedTexture{};
            loaded_texture.name = texture.name;
            arm::ensure(texture.imageIndex.has_value(), "not sure this will happen but maybe it will");
            loaded_texture.image_index = texture.imageIndex.value();
            loaded_texture.sampler_index =
                texture.samplerIndex.has_value() ? std::make_optional(texture.samplerIndex.value()) : std::nullopt;
            loaded_asset.textures.push_back(std::move(loaded_texture));
        }
    }
}

} // namespace pong
