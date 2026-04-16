#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include "fastgltf_primitives.h"

namespace pong
{

class FastGLTFWrapper
{
  public:
    FastGLTFWrapper();
    ~FastGLTFWrapper() = default;

    FastGLTFWrapper(const FastGLTFWrapper &) = delete;
    auto operator=(const FastGLTFWrapper &) -> FastGLTFWrapper & = delete;

    FastGLTFWrapper(FastGLTFWrapper &&) noexcept = default;
    auto operator=(FastGLTFWrapper &&) noexcept -> FastGLTFWrapper & = default;

    auto load(std::filesystem::path path) -> LoadedAsset;

  private:
    // NOT thread-safe
    ::fastgltf::Parser parser_;

    auto extract_images_(::fastgltf::Asset &data, LoadedAsset &loaded_asset) -> void;
    auto extract_samplers_(::fastgltf::Asset &data, LoadedAsset &loaded_asset) -> void;
    auto extract_textures_(::fastgltf::Asset &data, LoadedAsset &loaded_asset) -> void;
    auto extract_materials_(::fastgltf::Asset &data, LoadedAsset &loaded_asset) -> void;
    auto extract_meshes_(::fastgltf::Asset &data, LoadedAsset &loaded_asset) -> void;
    auto extract_nodes_(::fastgltf::Asset &data, LoadedAsset &loaded_asset) -> void;
    auto extract_scenes_(::fastgltf::Asset &data, LoadedAsset &loaded_asset) -> void;
};

}
