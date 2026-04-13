#include <cmath>
#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include "core/entity.h"
#include "graphics/glm_wrapper.h"

namespace pong
{

TEST(Entity, ConstructsWithExpectedState)
{
    auto initial = Transform{};
    initial.position = {1.0f, 2.0f, 3.0f};

    const auto handle = std::uint64_t{42};
    auto entity = Entity{"player", handle, initial};

    EXPECT_TRUE(entity.is_active());
    EXPECT_EQ(entity.name(), "player");
    EXPECT_EQ(entity.mesh_handle(), handle);
    EXPECT_FLOAT_EQ(entity.transform().position.x, 1.0f);
    EXPECT_FLOAT_EQ(entity.transform().position.y, 2.0f);
    EXPECT_FLOAT_EQ(entity.transform().position.z, 3.0f);
}

TEST(Entity, SettersUpdateTransform)
{
    auto entity = Entity{"ball", 7u, Transform{}};

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
    auto entity = Entity{"paddle", 9u, Transform{}};
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
    auto entity = Entity{"entity", 1u, Transform{}};
    EXPECT_TRUE(entity.is_active());

    entity.set_active(false);
    EXPECT_FALSE(entity.is_active());

    entity.set_active(true);
    EXPECT_TRUE(entity.is_active());
}

TEST(Entity, TranslateByAddsWorldOffset)
{
    auto entity = Entity{"entity", 1u, Transform{}};
    entity.set_position({1.0f, 2.0f, 3.0f});

    entity.translate_by({0.5f, -1.0f, 4.0f});

    EXPECT_FLOAT_EQ(entity.transform().position.x, 1.5f);
    EXPECT_FLOAT_EQ(entity.transform().position.y, 1.0f);
    EXPECT_FLOAT_EQ(entity.transform().position.z, 7.0f);
}

TEST(Entity, TranslateByIgnoresNonFiniteOffset)
{
    auto entity = Entity{"entity", 1u, Transform{}};
    entity.set_position({4.0f, 5.0f, 6.0f});

    const auto inf = std::numeric_limits<float>::infinity();
    entity.translate_by({inf, 1.0f, 1.0f});

    EXPECT_FLOAT_EQ(entity.transform().position.x, 4.0f);
    EXPECT_FLOAT_EQ(entity.transform().position.y, 5.0f);
    EXPECT_FLOAT_EQ(entity.transform().position.z, 6.0f);
}

TEST(Entity, SetPositionIgnoresNonFiniteInput)
{
    auto entity = Entity{"entity", 1u, Transform{}};
    entity.set_position({1.0f, 2.0f, 3.0f});

    const auto nan = std::numeric_limits<float>::quiet_NaN();
    entity.set_position({nan, 9.0f, 9.0f});

    EXPECT_FLOAT_EQ(entity.transform().position.x, 1.0f);
    EXPECT_FLOAT_EQ(entity.transform().position.y, 2.0f);
    EXPECT_FLOAT_EQ(entity.transform().position.z, 3.0f);
}

TEST(Entity, LocalTranslationAndLocalRotationPendingImplementation)
{
    GTEST_SKIP() << "translate_local() and rotate_local_by() are still pending implementation in Entity.";
}

} // namespace pong
