#pragma once

#include "glm_wrapper.h"

namespace pong
{

class Camera
{
  public:
    Camera(::glm::vec3 eye, ::glm::vec3 look_at, ::glm::vec3 up = {0.0f, 1.0f, 0.0f});
    Camera() = default;
    ~Camera() = default;

    Camera(const Camera &) = default;
    auto operator=(const Camera &) -> Camera & = default;
    Camera(Camera &&) noexcept = default;
    auto operator=(Camera &&) noexcept -> Camera & = default;

    auto translate(::glm::vec3 offset) -> void;
    auto set_position(::glm::vec3 position) -> void;

    auto set_view_target(::glm::vec3 target) -> void;
    auto get_view_matrix() const -> ::glm::mat4;

  private:
    ::glm::vec3 eye_{0.0f, 0.0f, 5.0f};
    ::glm::vec3 center_{0.0f, 0.0f, 0.0f};
    ::glm::vec3 up_{0.0f, 1.0f, 0.0f};

    auto validate_() -> void;
};

}
