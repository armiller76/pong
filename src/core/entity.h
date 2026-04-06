#pragma once

#include <string>
#include <string_view>

#include "graphics/glm_wrapper.h"
#include "graphics/mesh.h"
#include "math/transform.h"

namespace pong
{

class Entity
{
  public:
    Entity(std::string_view name, std::uint64_t mesh_handle, Transform transform);

    auto set_active(bool active) -> void;
    auto is_active() const -> bool;

    auto name() const -> std::string_view;

    auto mesh_handle() const -> std::uint64_t;

    auto transform() -> Transform &;
    auto transform() const -> const Transform &;

    auto set_position(::glm::vec3 position) -> void;
    auto translate(::glm::vec3 offset) -> void;

    auto set_scale(::glm::vec3 scale) -> void;
    auto scale(::glm::vec3 factor) -> void;

    auto set_rotation(::glm::quat rotation) -> void;
    auto rotate_by(::glm::vec3 radians) -> void;

  private:
    bool active_ = true;
    std::string name_;
    std::uint64_t mesh_handle_;
    Transform transform_;
};

}
