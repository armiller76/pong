#include "camera.h"

#include "glm/ext/vector_float3.hpp"
#include "glm_wrapper.h" // IWYU pragma: keep
#include "math/utils.h"

namespace pong
{

Camera::Camera(
    const ::glm::vec3 eye,
    const ::glm::vec3 look_at,
    const ::glm::vec3 up,
    const float fov,
    const float near_plane,
    const float far_plane)
    : eye_{eye}
    , center_{look_at}
    , up_{up}
    , fov_{fov}
    , near_plane_{near_plane}
    , far_plane_{far_plane}
{
    arm::log::debug("Camera constructor");

    validate_();
}

auto Camera::translate(const ::glm::vec3 offset) -> void
{
    if (!all_finite(offset))
    {
        arm::log::warn("camera: translate offset component infinite, dropping");
        return;
    }

    eye_ += offset;
    center_ += offset;
}

auto Camera::set_position(const ::glm::vec3 position) -> void
{
    if (!all_finite(position))
    {
        arm::log::warn("camera: set_position position component infinite, dropping");
        return;
    }

    auto offset = position - eye_;
    eye_ = position;
    center_ += offset;
}

auto Camera::get_position() const -> const ::glm::vec3
{
    return eye_;
}

auto Camera::set_view_target(const ::glm::vec3 target) -> void
{
    if (!all_finite(target))
    {
        arm::log::warn("camera: set_view_target target component infinite, dropping");
        return;
    }

    center_ = target;
    validate_();
}

auto Camera::get_view_matrix() const -> ::glm::mat4
{
    return ::glm::lookAt(eye_, center_, up_);
}

auto Camera::validate_() -> void
{
    if (up_ == ::glm::vec3{0.0f})
    {
        arm::log::warn("camera validation: all up components == 0, using default up = 0,1,0");
        up_ = {0.0f, 1.0f, 0.0f};
    }

    if (eye_ == center_)
    {
        arm::log::warn("camera validation: eye == center, using defaults eye=0,0,5; center=0,0,0");
        eye_ = {0.0f, 0.0f, 5.0f};
        center_ = {0.0f, 0.0f, 0.0f};
    }

    auto forward = center_ - eye_;
    auto cross = ::glm::cross(forward, up_);
    if (::glm::length(cross) < g_epsilon)
    {
        auto default_up = ::glm::vec3{0.0f, 1.0f, 0.0f};
        auto cross2 = ::glm::cross(forward, default_up);
        if (::glm::length(cross2) < g_epsilon)
        {
            arm::log::warn(
                "camera validation: up collinear with forward and forward ≈ default up, making up = side vector");
            up_ = {1.0f, 0.0f, 0.0f};
        }
        else
        {
            arm::log::warn("camera validation: up collinear with forward, using default up = 0,1,0");
            up_ = default_up;
        }
    }
}

auto Camera::camera_ubo(const float aspect) const -> UBO_Camera
{
    auto result = UBO_Camera{
        .view = get_view_matrix(),
        .proj = ::glm::perspective(fov_, aspect, near_plane_, far_plane_),
        .camera = {get_position(), 0.0f}};
    result.proj[1][1] *= -1.0f;
    return result;
}

} // namespace pong
