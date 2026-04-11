#include "camera.h"

#include "glm_wrapper.h"
#include "math/utils.h"

namespace pong
{

Camera::Camera(::glm::vec3 eye, ::glm::vec3 look_at, ::glm::vec3 up)
    : eye_{eye}
    , center_{look_at}
    , up_{up}
{
    arm::log::debug("Camera constructor");
    validate_();
}

auto Camera::translate(::glm::vec3 offset) -> void
{
    if (!all_finite(offset))
    {
        arm::log::warn("camera: translate offset component infinite, dropping");
        return;
    }

    eye_ += offset;
    center_ += offset;
}

auto Camera::set_position(::glm::vec3 position) -> void
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

auto Camera::set_view_target(::glm::vec3 target) -> void
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

} // namespace pong
