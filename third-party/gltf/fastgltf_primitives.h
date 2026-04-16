#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "graphics/glm_wrapper.h"
#include "graphics/vertex.h"
#include "math/transform.h"

namespace pong
{

enum class AlphaMode
{
    Opaque,
    Mask,
    Blend,
};

enum class MinFilterMode
{
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear,
};

enum class MagFilterMode
{
    Nearest,
    Linear,
};

enum class WrapMode
{
    Repeat,
    MirroredRepeat,
    ClampToEdge,
};

struct LoadedPrimitive
{
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    std::optional<std::size_t> material_index;
};

struct LoadedMesh
{
    std::string name;
    std::vector<LoadedPrimitive> primitives;
};

struct LoadedNode
{
    std::string name;
    Transform local_transform;
    std::optional<std::size_t> mesh_index;  // index into LoadedAsset::meshes
    std::vector<std::size_t> child_indices; // index into LoadedAsset::nodes
};

struct LoadedScene
{
    std::string name;
    std::vector<std::size_t> root_node_indices; // index into LoadedAsset::nodes
};

struct LoadedMaterial
{
    std::string name;
    ::glm::vec4 base_color_factor;
    std::optional<std::size_t> base_color_texture_index; // index into LoadedAsset::textures
    float metallic_factor;
    float roughness_factor;
    std::optional<std::size_t> metallic_roughness_texture_index; // index into LoadedAsset::textures
    std::optional<std::size_t> normal_texture;                   // index into LoadedAsset::textures
    AlphaMode alpha_mode;
};

struct LoadedTexture
{
    std::string name;
    std::size_t image_index;                  // index into LoadedAsset::images
    std::optional<std::size_t> sampler_index; // index into LoadedAsset::samplers
};

struct LoadedImage
{
    std::string name;
    // can store a path to a file, or the actual raw bytes
    std::variant<std::filesystem::path, std::vector<std::uint8_t>> data;
};

struct LoadedSampler
{
    MagFilterMode mag_filter;
    MinFilterMode min_filter;
    WrapMode wrap_u;
    WrapMode wrap_v;
};

struct LoadedAsset
{
    std::optional<std::size_t> default_scene_index;
    std::vector<LoadedScene> scenes;
    std::vector<LoadedNode> nodes;
    std::vector<LoadedMesh> meshes;
    std::vector<LoadedMaterial> materials;
    std::vector<LoadedTexture> textures;
    std::vector<LoadedImage> images;
    std::vector<LoadedSampler> samplers;
};

}
