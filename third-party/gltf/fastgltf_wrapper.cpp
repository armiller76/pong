#include "fastgltf_wrapper.h"

#include <cstdint>
#include <filesystem>
#include <ranges>
#include <string_view>
#include <vector>

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

#include "fastgltf_glm_traits.h"
#include "fastgltf_primitives.h"
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
    : parser_{
          ::fastgltf::Extensions::KHR_mesh_quantization | ::fastgltf::Extensions::KHR_texture_transform
          | ::fastgltf::Extensions::KHR_materials_unlit}
{
    arm::log::debug("FastGLTFWrapper constructor");
}

auto FastGLTFWrapper::load(std::filesystem::path path) -> LoadedAsset
{
    auto data = ::fastgltf::GltfDataBuffer::FromPath(path);
    arm::ensure(
        data.error() == ::fastgltf::Error::None,
        "FastGLTF: unable to load {} ({})",
        path.filename().string(),
        ::fastgltf::getErrorMessage(data.error()));

    auto asset = parser_.loadGltf(
        data.get(),
        path.parent_path(),
        ::fastgltf::Options::LoadGLBBuffers | ::fastgltf::Options::LoadExternalBuffers
            | ::fastgltf::Options::GenerateMeshIndices);
    arm::ensure(
        asset.error() == ::fastgltf::Error::None,
        "FastGLTF: unable to load {} ({})",
        path.filename().string(),
        ::fastgltf::getErrorMessage(asset.error()));

    // if we made it this far, we have parsed the gltf data into 'asset'
    // for now, let's see if we can extract a single mesh from assets/gltf/box/box.glb
    auto pong_asset = LoadedAsset{};

    extract_images_(asset.get(), pong_asset);
    extract_samplers_(asset.get(), pong_asset);
    extract_textures_(asset.get(), pong_asset);
    extract_materials_(asset.get(), pong_asset);
    extract_meshes_(asset.get(), pong_asset);
    extract_nodes_(asset.get(), pong_asset);
    extract_scenes_(asset.get(), pong_asset);

    return pong_asset;
}

auto FastGLTFWrapper::extract_images_(::fastgltf::Asset &data, LoadedAsset &loaded_asset) -> void
{
    if (data.images.empty())
    {
        return;
    }
    else
    {
        throw arm::Exception("extracting images from gltf data is not yet supported.");
    }
}

auto FastGLTFWrapper::extract_samplers_(::fastgltf::Asset &data, LoadedAsset &loaded_asset) -> void
{
    if (data.samplers.empty())
    {
        return;
    }
    else
    {
        throw arm::Exception("extracting samplers from gltf data is not yet supported.");
    }
}

auto FastGLTFWrapper::extract_textures_(::fastgltf::Asset &data, LoadedAsset &loaded_asset) -> void
{
    if (data.textures.empty())
    {
        return;
    }
    else
    {
        throw arm::Exception("extracting textures from gltf data is not yet supported.");
    }
}

auto FastGLTFWrapper::extract_materials_(::fastgltf::Asset &data, LoadedAsset &loaded_asset) -> void
{
    if (data.materials.empty())
    {
        return;
    }
    else
    {
        arm::log::error("extracting materials from gltf data is not yet supported.");
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

                    fill_attribute<::glm::vec3>(
                        data,
                        primitive,
                        "COLOR",
                        loaded_primitive.vertices,
                        {1.0f, 1.0f, 1.0f},
                        [&](pong::Vertex &v, const ::glm::vec3 &value) { v.color = value; });

                    auto indices_accessor_index = primitive.indicesAccessor;
                    arm::ensure(indices_accessor_index.has_value(), "fastgltf error: unable to find vertex indices");

                    ::fastgltf::iterateAccessor<std::uint32_t>(
                        data,
                        data.accessors[indices_accessor_index.value()],
                        [&](std::uint32_t index) { loaded_primitive.indices.push_back(index); });

                    loaded_mesh.primitives.push_back(std::move(loaded_primitive));
                }
            }
            loaded_asset.meshes.push_back(std::move(loaded_mesh));
        }
    }
}

auto FastGLTFWrapper::extract_nodes_(::fastgltf::Asset &data, LoadedAsset &loaded_asset) -> void
{
}

auto FastGLTFWrapper::extract_scenes_(::fastgltf::Asset &data, LoadedAsset &loaded_asset) -> void
{
}

} // namespace pong::fastgltf
