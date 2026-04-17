#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>

#include <gtest/gtest.h>

#include "core/entity.h"
#include "core/resource_handles.h"
#include "graphics/glm_wrapper.h"
#include "graphics/model.h"
#include "graphics/renderable.h"

namespace pong
{
using namespace std::literals;

TEST(Entity, ConstructsWithExpectedState)
{
    auto initial = Transform{};
    initial.position = {1.0f, 2.0f, 3.0f};

    const auto handle = MeshHandle{42};
    const auto renderable = Renderable{handle, std::nullopt};
    const auto model = Model{"player_model", {renderable}};
    auto entity = Entity{"player", model, initial};

    EXPECT_TRUE(entity.is_active());
    EXPECT_EQ(entity.name(), "player");
    EXPECT_EQ(entity.model().name, "player_model");
    EXPECT_EQ(entity.model().renderables[0].mesh_handle, handle);
    EXPECT_FLOAT_EQ(entity.transform().position.x, 1.0f);
    EXPECT_FLOAT_EQ(entity.transform().position.y, 2.0f);
    EXPECT_FLOAT_EQ(entity.transform().position.z, 3.0f);
}

TEST(Entity, SettersUpdateTransform)
{
    const auto handle = MeshHandle{42};
    const auto renderable = Renderable{handle, std::nullopt};
    const auto model = Model{"model", {renderable}};
    auto entity = Entity{"ball", model, Transform{}};

    entity.set_position({-0.5f, 0.25f, 0.75f});
    entity.set_scale({2.0f, 3.0f, 4.0f});

    const auto q = ::glm::quat(::glm::vec3{0.0f, 0.0f, 0.5f});
    entity.set_rotation(q);

    EXPECT_FLOAT_EQ(entity.transform().position.x, -0.5f);
    EXPECT_FLOAT_EQ(entity.transform().position.y, 0.25f);
    EXPECT_FLOAT_EQ(entity.transform().position.z, 0.75f);

    EXPECT_FLOAT_EQ(entity.transform().scale.x, 2.0f);
    EXPECT_FLOAT_EQ(entity.transform().scale.y, 3.0f);
    EXPECT_FLOAT_EQ(entity.transform().scale.z, 4.0f);

    const auto dot = ::glm::dot(entity.transform().rotation, q);
    EXPECT_NEAR(std::abs(dot), 1.0f, 1e-5f);
}

TEST(Entity, ScaleAndRotateByAreComposed)
{
    const auto handle = MeshHandle{42};
    const auto renderable = Renderable{handle, std::nullopt};
    const auto model = Model{"model", {renderable}};
    auto entity = Entity{"paddle", model, Transform{}};
    entity.set_scale({2.0f, 3.0f, 4.0f});

    entity.scale_by({0.5f, 2.0f, 1.5f});

    EXPECT_FLOAT_EQ(entity.transform().scale.x, 1.0f);
    EXPECT_FLOAT_EQ(entity.transform().scale.y, 6.0f);
    EXPECT_FLOAT_EQ(entity.transform().scale.z, 6.0f);

    const auto start = ::glm::quat(::glm::vec3{0.1f, -0.3f, 0.2f});
    const auto delta = ::glm::vec3{0.25f, 0.0f, -0.5f};
    entity.set_rotation(start);
    entity.rotate_by(delta);

    const auto expected = ::glm::normalize(::glm::quat(delta) * start);
    const auto dot = ::glm::dot(entity.transform().rotation, expected);
    EXPECT_NEAR(std::abs(dot), 1.0f, 1e-5f);
}

TEST(Entity, ActiveStateToggles)
{
    const auto handle = MeshHandle{42};
    const auto renderable = Renderable{handle, std::nullopt};
    const auto model = Model{"model", {renderable}};
    auto entity = Entity{"entity", model, Transform{}};
    EXPECT_TRUE(entity.is_active());

    entity.set_active(false);
    EXPECT_FALSE(entity.is_active());

    entity.set_active(true);
    EXPECT_TRUE(entity.is_active());
}

TEST(Entity, TranslateByAddsWorldOffset)
{
    const auto handle = MeshHandle{42};
    const auto renderable = Renderable{handle, std::nullopt};
    const auto model = Model{"model", {renderable}};
    auto entity = Entity{"entity", model, Transform{}};
    entity.set_position({1.0f, 2.0f, 3.0f});

    entity.translate_by({0.5f, -1.0f, 4.0f});

    EXPECT_FLOAT_EQ(entity.transform().position.x, 1.5f);
    EXPECT_FLOAT_EQ(entity.transform().position.y, 1.0f);
    EXPECT_FLOAT_EQ(entity.transform().position.z, 7.0f);
}

TEST(Entity, TranslateByIgnoresNonFiniteOffset)
{
    const auto handle = MeshHandle{42};
    const auto renderable = Renderable{handle, std::nullopt};
    const auto model = Model{"model", {renderable}};
    auto entity = Entity{"entity", model, Transform{}};
    entity.set_position({4.0f, 5.0f, 6.0f});

    const auto inf = std::numeric_limits<float>::infinity();
    entity.translate_by({inf, 1.0f, 1.0f});

    EXPECT_FLOAT_EQ(entity.transform().position.x, 4.0f);
    EXPECT_FLOAT_EQ(entity.transform().position.y, 5.0f);
    EXPECT_FLOAT_EQ(entity.transform().position.z, 6.0f);
}

TEST(Entity, SetPositionIgnoresNonFiniteInput)
{
    const auto handle = MeshHandle{42};
    const auto renderable = Renderable{handle, std::nullopt};
    const auto model = Model{"model", {renderable}};
    auto entity = Entity{"entity", model, Transform{}};
    entity.set_position({1.0f, 2.0f, 3.0f});

    const auto nan = std::numeric_limits<float>::quiet_NaN();
    entity.set_position({nan, 9.0f, 9.0f});

    EXPECT_FLOAT_EQ(entity.transform().position.x, 1.0f);
    EXPECT_FLOAT_EQ(entity.transform().position.y, 2.0f);
    EXPECT_FLOAT_EQ(entity.transform().position.z, 3.0f);
}

TEST(Entity, LocalTranslationPendingImplementation)
{
    GTEST_SKIP() << "translate_local() is still pending implementation in Entity.";
}

TEST(Entity, LocalRotationPendingImplementation)
{
    GTEST_SKIP() << "rotate_local_by() is still pending implementation in Entity.";
}

} // namespace pong
