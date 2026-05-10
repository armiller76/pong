#pragma once

#include "engine/ubo.h"
#include "glm_wrapper.h" // IWYU pragma: keep

namespace pong
{

class Camera
{
  public:
    Camera(
        const ::glm::vec3 position,
        const ::glm::vec3 look_at_point,
        const ::glm::vec3 up_direction,
        const float vertical_fov_radians,
        const float near_clip,
        const float far_clip);
    ~Camera() = default;

    Camera(const Camera &) = default;
    auto operator=(const Camera &) -> Camera & = default;
    Camera(Camera &&) noexcept = default;
    auto operator=(Camera &&) noexcept -> Camera & = default;

    auto translate_by(const ::glm::vec3 delta_world) -> void;
    auto set_position(const ::glm::vec3 position) -> void;
    auto get_position() const -> const ::glm::vec3;

    auto get_right_direction() const -> const ::glm::vec3;
    auto get_forward_direction() const -> const ::glm::vec3;

    auto set_look_at_point(const ::glm::vec3 look_at_point) -> void;
    auto get_look_at_point() const -> const ::glm::vec3;

    auto get_view_matrix() const -> ::glm::mat4;

    auto camera_ubo(const float aspect) const -> UBO_Camera;

  private:
    ::glm::vec3 position_{0.0f, 0.0f, 5.0f};
    ::glm::vec3 look_at_point_{0.0f, 0.0f, 0.0f};
    ::glm::vec3 up_direction_{0.0f, 1.0f, 0.0f};
    ::glm::vec3 forward_direction_{0.0f, 0.0f, -1.0f};
    ::glm::vec3 right_direction_{1.0f, 0.0f, 0.0f};

    float vertical_fov_radians_;
    float near_clip_;
    float far_clip_;

    auto validate_and_rebuild_axes_() -> void;
};

}
