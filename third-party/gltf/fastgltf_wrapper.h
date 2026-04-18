#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

#include "fastgltf_primitives.h"

namespace fastgltf
{
class Parser;
class Asset;
}

namespace pong
{

class FastGLTFWrapper
{
  public:
    FastGLTFWrapper();
    ~FastGLTFWrapper();

    FastGLTFWrapper(const FastGLTFWrapper &) = delete;
    auto operator=(const FastGLTFWrapper &) -> FastGLTFWrapper & = delete;

    FastGLTFWrapper(FastGLTFWrapper &&) noexcept = delete;
    auto operator=(FastGLTFWrapper &&) noexcept -> FastGLTFWrapper & = delete;

    auto load(std::filesystem::path path) -> LoadedAsset;

  private:
    std::unique_ptr<::fastgltf::Parser> parser_;

    auto extract_scenes_(::fastgltf::Asset &asset, LoadedAsset &loaded_asset) -> void;
    auto extract_nodes_(::fastgltf::Asset &asset, LoadedAsset &loaded_asset) -> void;
    auto extract_materials_(::fastgltf::Asset &asset, LoadedAsset &loaded_asset) -> void;
    auto extract_meshes_(::fastgltf::Asset &asset, LoadedAsset &loaded_asset) -> void;
    auto extract_images_(::fastgltf::Asset &asset, LoadedAsset &loaded_asset) -> void;
    auto extract_samplers_(::fastgltf::Asset &asset, LoadedAsset &loaded_asset) -> void;
    auto extract_textures_(::fastgltf::Asset &asset, LoadedAsset &loaded_asset) -> void;
};

}
