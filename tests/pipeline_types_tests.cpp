#include <functional>

#include <gtest/gtest.h>

#include "engine/vulkan/vulkan_pipeline_types.h"

namespace pong
{

TEST(PipelineTypes, ShaderFeatureBitOpsRoundTrip)
{
    auto features = ShaderFeature::None;
    EXPECT_FALSE(any(features));

    features |= ShaderFeature::NormalMap;
    EXPECT_TRUE(any(features));
    EXPECT_TRUE(has(features, ShaderFeature::NormalMap));
    EXPECT_FALSE(has(features, ShaderFeature::AlphaTest));

    features |= ShaderFeature::AlphaTest;
    EXPECT_TRUE(has(features, ShaderFeature::NormalMap));
    EXPECT_TRUE(has(features, ShaderFeature::AlphaTest));
}

TEST(PipelineTypes, PipelineKeyPackChangesAcrossFields)
{
    auto key_a = PipelineKey{
        .pass_type = PassType::Main,
        .alpha_mode = AlphaMode::Opaque,
        .raster_state = RasterState::Default,
        .vertex_input = VertexInput::StaticMesh,
        .shader_features = ShaderFeature::None};

    auto key_b = PipelineKey{
        .pass_type = PassType::Main,
        .alpha_mode = AlphaMode::Blend,
        .raster_state = RasterState::Default,
        .vertex_input = VertexInput::StaticMesh,
        .shader_features = ShaderFeature::None};

    auto key_c = PipelineKey{
        .pass_type = PassType::Light,
        .alpha_mode = AlphaMode::Opaque,
        .raster_state = RasterState::Default,
        .vertex_input = VertexInput::StaticMesh,
        .shader_features = ShaderFeature::NormalMap};

    EXPECT_NE(key_a.pack(), key_b.pack());
    EXPECT_NE(key_a.pack(), key_c.pack());
}

TEST(PipelineTypes, PipelineKeyHashMatchesEquality)
{
    auto left = PipelineKey{
        .pass_type = PassType::Main,
        .alpha_mode = AlphaMode::Mask,
        .raster_state = RasterState::Wireframe,
        .vertex_input = VertexInput::Debug,
        .shader_features = ShaderFeature::AlphaTest};

    auto right = left;

    EXPECT_TRUE(left == right);
    EXPECT_EQ(std::hash<PipelineKey>{}(left), std::hash<PipelineKey>{}(right));
}

TEST(PipelineTypes, NamePipelineProducesStableReadableToken)
{
    auto key = PipelineKey{
        .pass_type = PassType::Main,
        .alpha_mode = AlphaMode::Blend,
        .raster_state = RasterState::Double,
        .vertex_input = VertexInput::StaticMesh,
        .shader_features = ShaderFeature::NormalMap | ShaderFeature::AlphaTest};

    EXPECT_EQ(name_pipeline(key), "main_double_blend_staticmesh_3");
}

} // namespace pong
