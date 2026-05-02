#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "graphics/camera.h"

namespace pong
{

namespace
{

constexpr auto kDefaultEye = ::glm::vec3{0.0f, 0.0f, 5.0f};
constexpr auto kDefaultCenter = ::glm::vec3{0.0f, 0.0f, 0.0f};
constexpr auto kDefaultUp = ::glm::vec3{0.0f, 1.0f, 0.0f};
constexpr auto kDefaultFov = ::glm::radians(45.0f);
constexpr auto kDefaultNear = 0.1f;
constexpr auto kDefaultFar = 100.0f;

auto make_camera(
    const ::glm::vec3 eye = kDefaultEye,
    const ::glm::vec3 center = kDefaultCenter,
    const ::glm::vec3 up = kDefaultUp,
    const float fov = kDefaultFov,
    const float near_plane = kDefaultNear,
    const float far_plane = kDefaultFar) -> Camera
{
    return Camera{eye, center, up, fov, near_plane, far_plane};
}

auto expect_vec3_near(const ::glm::vec3 &a, const ::glm::vec3 &b, float eps = 1e-5f) -> void
{
    EXPECT_NEAR(a.x, b.x, eps);
    EXPECT_NEAR(a.y, b.y, eps);
    EXPECT_NEAR(a.z, b.z, eps);
}

// A view matrix maps the eye position to the view-space origin.
auto eye_in_view_space(const ::glm::mat4 &view, ::glm::vec3 eye) -> ::glm::vec3
{
    const auto result = view * ::glm::vec4{eye, 1.0f};
    return {result.x, result.y, result.z};
}

} // namespace

TEST(Camera, DefaultEyeMapsToCameraOrigin)
{
    const auto cam = make_camera();
    expect_vec3_near(eye_in_view_space(cam.get_view_matrix(), {0.0f, 0.0f, 5.0f}), {0.0f, 0.0f, 0.0f});
}

TEST(Camera, ConstructedWithArgsMovesEyeToOrigin)
{
    const auto cam = make_camera({3.0f, 4.0f, 0.0f}, {0.0f, 0.0f, 0.0f});
    expect_vec3_near(eye_in_view_space(cam.get_view_matrix(), {3.0f, 4.0f, 0.0f}), {0.0f, 0.0f, 0.0f});
}

TEST(Camera, TranslatePreservesLookDirection)
{
    auto cam = make_camera({0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, 0.0f});

    // The look direction before translate is (0,0,-1) (eye toward center).
    // After translating by (2,0,0), both eye and center shift, so look direction
    // must remain (0,0,-1). Verify by checking the view matrix's Z-row (forward).
    const auto view_before = cam.get_view_matrix();
    cam.translate({2.0f, 0.0f, 0.0f});
    const auto view_after = cam.get_view_matrix();

    // Row 2 (Z-row) encodes the forward direction; it should be unchanged.
    for (int col = 0; col < 3; ++col)
    {
        EXPECT_NEAR(view_after[col][2], view_before[col][2], 1e-5f);
    }
}

TEST(Camera, TranslateMovesEyeByOffset)
{
    auto cam = make_camera({0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, 0.0f});
    cam.translate({1.0f, 2.0f, -3.0f});

    // After the translate, the new eye should map to view-space origin.
    expect_vec3_near(eye_in_view_space(cam.get_view_matrix(), {1.0f, 2.0f, 2.0f}), {0.0f, 0.0f, 0.0f});
}

TEST(Camera, SetPositionMovesEyeAndPreservesLookDirection)
{
    auto cam = make_camera({0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, 0.0f});

    const auto view_before = cam.get_view_matrix();
    cam.set_position({10.0f, 0.0f, 5.0f});
    const auto view_after = cam.get_view_matrix();

    // Z-row (look direction) should still be the same.
    for (int col = 0; col < 3; ++col)
    {
        EXPECT_NEAR(view_after[col][2], view_before[col][2], 1e-5f);
    }

    // The new eye must map to view-space origin.
    expect_vec3_near(eye_in_view_space(view_after, {10.0f, 0.0f, 5.0f}), {0.0f, 0.0f, 0.0f});
}

TEST(Camera, SetViewTargetChangesLookDirection)
{
    auto cam = make_camera({0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, 0.0f});

    const auto view_before = cam.get_view_matrix();
    // Point camera at something clearly off-axis.
    cam.set_view_target({5.0f, 0.0f, 0.0f});
    const auto view_after = cam.get_view_matrix();

    // Z-rows must differ after retargeting.
    bool changed = false;
    for (int col = 0; col < 3; ++col)
    {
        if (std::abs(view_after[col][2] - view_before[col][2]) > 1e-3f)
        {
            changed = true;
        }
    }
    EXPECT_TRUE(changed);
}

TEST(Camera, GetViewMatrixMatchesGlmLookAt)
{
    const auto eye = ::glm::vec3{1.0f, 2.0f, 3.0f};
    const auto center = ::glm::vec3{4.0f, 5.0f, 6.0f};
    const auto up = ::glm::vec3{0.0f, 1.0f, 0.0f};

    const auto cam = make_camera(eye, center, up);
    const auto expected = ::glm::lookAt(eye, center, up);
    const auto actual = cam.get_view_matrix();

    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            EXPECT_NEAR(actual[col][row], expected[col][row], 1e-5f);
        }
    }
}

} // namespace pong
