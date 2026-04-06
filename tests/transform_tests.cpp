#include <gtest/gtest.h>

#include "graphics/glm_wrapper.h"
#include "math/transform.h"

namespace pong
{

namespace
{

auto expect_mat4_near(const ::glm::mat4 &a, const ::glm::mat4 &b, float eps = 1e-5f) -> void
{
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            EXPECT_NEAR(a[col][row], b[col][row], eps);
        }
    }
}

} // namespace

TEST(Transform, DefaultConvertsToIdentity)
{
    const auto t = Transform{};
    const auto m = ::glm::mat4(t);

    expect_mat4_near(m, ::glm::mat4(1.0f));
}

TEST(Transform, TranslationAppearsInMatrix)
{
    auto t = Transform{};
    t.position = {3.0f, -2.0f, 0.5f};

    const auto m = ::glm::mat4(t);

    EXPECT_FLOAT_EQ(m[3][0], 3.0f);
    EXPECT_FLOAT_EQ(m[3][1], -2.0f);
    EXPECT_FLOAT_EQ(m[3][2], 0.5f);
}

TEST(Transform, ScaleAffectsDiagonal)
{
    auto t = Transform{};
    t.scale = {2.0f, 3.0f, 4.0f};

    const auto m = ::glm::mat4(t);

    EXPECT_FLOAT_EQ(m[0][0], 2.0f);
    EXPECT_FLOAT_EQ(m[1][1], 3.0f);
    EXPECT_FLOAT_EQ(m[2][2], 4.0f);
}

TEST(Transform, RotationRotatesBasisVector)
{
    auto t = Transform{};
    t.rotation = ::glm::quat(::glm::vec3{0.0f, 0.0f, ::glm::half_pi<float>()});

    const auto m = ::glm::mat4(t);
    const auto x_axis = ::glm::vec4{1.0f, 0.0f, 0.0f, 0.0f};
    const auto rotated = m * x_axis;

    EXPECT_NEAR(rotated.x, 0.0f, 1e-5f);
    EXPECT_NEAR(rotated.y, 1.0f, 1e-5f);
    EXPECT_NEAR(rotated.z, 0.0f, 1e-5f);
}

} // namespace pong
