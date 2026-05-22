#include "free_look_camera.h"

#include <algorithm>
#include <cstdint>
#include <numbers>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "math/utils.h"
#include "utils/error.h"

namespace
{

auto create_forward(float pitch, float yaw) -> ::glm::vec3
{
    return ::glm::normalize(
        ::glm::vec3{std::cos(yaw) * std::cos(pitch), std::sin(pitch), std::sin(yaw) * std::cos(pitch)});
}

}

namespace pong
{

FreeLookCamera::FreeLookCamera(
    const ::glm::vec3 position,
    const ::glm::vec3 world_up,
    float vertical_fov_radians,
    float width,
    float height,
    const float near_clip,
    const float far_clip)
    : vertical_fov_radians_{vertical_fov_radians}
    , width_{width}
    , height_{height}
    , near_clip_{near_clip}
    , far_clip_{far_clip}
    , pitch_{0.0f}
    , yaw_{-std::numbers::pi_v<float> / 2.0f}
    , world_up_direction_{world_up}
    , forward_direction_{}
    , up_direction_{}
    , right_direction_{}
    , camera_data_{{}, {}, position}
{
    arm::log::debug("FreeLookCamera constructor");
    resize(static_cast<std::uint32_t>(width_), static_cast<std::uint32_t>(height_));
    adjust_pitch(0.0f);
}

auto FreeLookCamera::translate_by(const ::glm::vec3 delta_world) -> void
{
    if (!all_finite(delta_world))
    {
        arm::log::warn("camera: translate_by offset vector component infinite, ignoring");
        return;
    }

    camera_data_.position += delta_world;
    update_view_();
}

auto FreeLookCamera::adjust_pitch(float radians) -> void
{
    if (::glm::isinf(radians))
    {
        arm::log::warn("camera: adjust_pitch angle infinite, ignoring");
        return;
    }

    pitch_ += radians;
    pitch_ = std::clamp(
        pitch_,
        -std::numbers::pi_v<float> / 2.0f + pong::g_epsilon,
        std::numbers::pi_v<float> / 2.0f - pong::g_epsilon);
    update_all_();
}

auto FreeLookCamera::adjust_yaw(float radians) -> void
{
    if (::glm::isinf(radians))
    {
        arm::log::warn("camera: adjust_yaw angle infinite, ignoring");
        return;
    }

    const auto pi_x2 = std::numbers::pi_v<float> * 2.0f;
    yaw_ += radians;
    yaw_ = std::fmodf(yaw_, pi_x2);
    if (yaw_ < 0)
    {
        yaw_ += pi_x2;
    }
    update_all_();
}

auto FreeLookCamera::set_fov(float radians) -> void
{
    if (::glm::isinf(radians))
    {
        arm::log::warn("camera: set_fov angle infinite, ignoring");
        return;
    }

    vertical_fov_radians_ = radians;
    update_all_();
}

auto FreeLookCamera::set_world_up(const ::glm::vec3 up) -> void
{
    if (!all_finite(up))
    {
        arm::log::warn("camera: set_world_up vector component infinite, ignoring");
        return;
    }

    world_up_direction_ = up;
    update_all_();
}

auto FreeLookCamera::resize(std::uint32_t width, std::uint32_t height) -> void
{
    arm::ensure(width != 0 && height != 0, "invalid resize w:{} h:{}", width, height);
    width_ = static_cast<float>(width);
    height_ = static_cast<float>(height);

    camera_data_.projection_matrix = ::glm::perspective(vertical_fov_radians_, width_ / height_, near_clip_, far_clip_);
    camera_data_.projection_matrix[1][1] *= -1;
}

auto FreeLookCamera::get_world_up() const -> const ::glm::vec3
{
    return world_up_direction_;
}

auto FreeLookCamera::get_forward_direction() const -> const ::glm::vec3
{
    return forward_direction_;
}

auto FreeLookCamera::get_up_direction() const -> const ::glm::vec3
{
    return up_direction_;
}

auto FreeLookCamera::get_right_direction() const -> const ::glm::vec3
{
    return right_direction_;
}

auto FreeLookCamera::get_fov() const -> float
{
    return vertical_fov_radians_;
}

auto FreeLookCamera::get_width() const -> float
{
    return width_;
}

auto FreeLookCamera::get_height() const -> float
{
    return height_;
}

auto FreeLookCamera::get_aspect() const -> float
{
    return width_ / height_;
}

auto FreeLookCamera::get_near_clip() const -> float
{
    return near_clip_;
}

auto FreeLookCamera::get_far_clip() const -> float
{
    return far_clip_;
}

auto FreeLookCamera::get_position() const -> const ::glm::vec3
{
    return camera_data_.position;
}

auto FreeLookCamera::get_camera_ubo() const -> UBO_Camera
{
    return {
        .view = camera_data_.view_matrix,
        .proj = camera_data_.projection_matrix,
        .camera = {camera_data_.position, 0.0f},
    };
}

auto FreeLookCamera::update_all_() -> void
{
    recalculate_vectors_();
    update_view_();
}

auto FreeLookCamera::recalculate_vectors_() -> void
{
    forward_direction_ = create_forward(pitch_, yaw_);
    right_direction_ = ::glm::normalize(::glm::cross(forward_direction_, world_up_direction_));
    up_direction_ = ::glm::normalize(::glm::cross(right_direction_, forward_direction_));
}

auto FreeLookCamera::update_view_() -> void
{
    camera_data_.view_matrix =
        ::glm::lookAt(camera_data_.position, camera_data_.position + forward_direction_, up_direction_);
}

} // namespace pong
