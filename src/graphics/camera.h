#pragma once

#include "engine/ubo.h"
#include "glm_wrapper.h" // IWYU pragma: keep

namespace pong
{

class Camera
{
  public:
    Camera(
        const ::glm::vec3 eye,
        const ::glm::vec3 look_at,
        const ::glm::vec3 up,
        const float fov,
        const float near_plane,
        const float far_plane);
    Camera() = default;
    ~Camera() = default;

    Camera(const Camera &) = default;
    auto operator=(const Camera &) -> Camera & = default;
    Camera(Camera &&) noexcept = default;
    auto operator=(Camera &&) noexcept -> Camera & = default;

    auto translate(const ::glm::vec3 offset) -> void;
    auto set_position(const ::glm::vec3 position) -> void;
    auto get_position() const -> const ::glm::vec3;

    auto set_view_target(const ::glm::vec3 target) -> void;
    auto get_view_matrix() const -> ::glm::mat4;

    auto camera_ubo(const float aspect) const -> UBO_Camera;

  private:
    ::glm::vec3 eye_{0.0f, 0.0f, 5.0f};
    ::glm::vec3 center_{0.0f, 0.0f, 0.0f};
    ::glm::vec3 up_{0.0f, 1.0f, 0.0f};

    float fov_;
    float near_plane_;
    float far_plane_;

    auto validate_() -> void;
};

}
