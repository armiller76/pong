#include "camera.h"

#include "glm/geometric.hpp"
#include "glm_wrapper.h" // IWYU pragma: keep
#include "math/utils.h"

namespace pong
{

Camera::Camera(
    const ::glm::vec3 position,
    const ::glm::vec3 look_at_point,
    const ::glm::vec3 up_direction,
    const float vertical_fov_radians,
    const float near_clip,
    const float far_clip)
    : position_{position}
    , look_at_point_{look_at_point}
    , up_direction_{up_direction}
    , vertical_fov_radians_{vertical_fov_radians}
    , near_clip_{near_clip}
    , far_clip_{far_clip}
{
    arm::log::debug("Camera constructor");

    validate_and_rebuild_axes_();
}

auto Camera::translate_by(const ::glm::vec3 delta_world) -> void
{
    if (!all_finite(delta_world))
    {
        arm::log::warn("camera: translate offset component infinite, dropping");
        return;
    }

    position_ += delta_world;
    look_at_point_ += delta_world;
    validate_and_rebuild_axes_();
}

auto Camera::set_position(const ::glm::vec3 position) -> void
{
    if (!all_finite(position))
    {
        arm::log::warn("camera: set_position position component infinite, dropping");
        return;
    }

    const auto offset = position - position_;
    position_ = position;
    look_at_point_ += offset;
    validate_and_rebuild_axes_();
}

auto Camera::get_position() const -> const ::glm::vec3
{
    return position_;
}

auto Camera::get_right_direction() const -> const ::glm::vec3
{
    return right_direction_;
}

auto Camera::get_forward_direction() const -> const ::glm::vec3
{
    return forward_direction_;
}

auto Camera::set_look_at_point(const ::glm::vec3 look_at_point) -> void
{
    if (!all_finite(look_at_point))
    {
        arm::log::warn("camera: set_look_at_point invalid, dropping");
        return;
    }

    look_at_point_ = look_at_point;
    validate_and_rebuild_axes_();
}

auto Camera::get_look_at_point() const -> const ::glm::vec3
{
    return look_at_point_;
}

auto Camera::get_view_matrix() const -> ::glm::mat4
{
    return ::glm::lookAt(position_, look_at_point_, up_direction_);
}

auto Camera::validate_and_rebuild_axes_() -> void
{
    if (up_direction_ == ::glm::vec3{0.0f})
    {
        arm::log::warn("camera validation: all up components == 0, using default up = 0,1,0");
        up_direction_ = {0.0f, 1.0f, 0.0f};
    }

    if (position_ == look_at_point_)
    {
        arm::log::warn(
            "camera validation: position can't equal look_at_point, using defaults pos=0,0,5; look_at=0,0,0");
        position_ = {0.0f, 0.0f, 5.0f};
        look_at_point_ = {0.0f, 0.0f, 0.0f};
    }

    const auto new_forward = look_at_point_ - position_;
    if (!(::glm::length(new_forward) < g_epsilon))
    {
        forward_direction_ = new_forward / ::glm::length(new_forward);
    }
    else
    {
        arm::log::warn("camera validation: length of forward direction too small, reusing last forward_dir");
    }

    const auto new_right = ::glm::cross(forward_direction_, up_direction_);
    if (::glm::length(new_right) < g_epsilon)
    {
        const auto default_up = ::glm::vec3{0.0f, 1.0f, 0.0f};
        const auto cross2 = ::glm::cross(forward_direction_, default_up);
        if (::glm::length(cross2) < g_epsilon)
        {
            arm::log::warn(
                "camera validation: up collinear with forward and forward ≈ default up, making up = side vector");
            up_direction_ = {1.0f, 0.0f, 0.0f};
        }
        else
        {
            arm::log::warn("camera validation: up collinear with forward, using default up = 0,1,0");
            up_direction_ = default_up;
        }
        right_direction_ = ::glm::normalize(::glm::cross(forward_direction_, up_direction_));
    }
    else
    {
        right_direction_ = ::glm::normalize(new_right);
    }

    up_direction_ = ::glm::normalize(::glm::cross(right_direction_, forward_direction_));
}

auto Camera::camera_ubo(const float aspect) const -> UBO_Camera
{
    auto result = UBO_Camera{
        .view = get_view_matrix(),
        .proj = ::glm::perspective(vertical_fov_radians_, aspect, near_clip_, far_clip_),
        .camera = {get_position(), 0.0f}};
    result.proj[1][1] *= -1.0f;
    return result;
}

} // namespace pong
