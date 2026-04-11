#pragma once

#include <cstdint>
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

    // Position and translation are in world space unless the method name says local.
    // Rotation quaternions are stored normalized.

    // Sets absolute world position.
    auto set_position(::glm::vec3 position) -> void;

    // Sets absolute scale (component-wise).
    auto set_scale(::glm::vec3 scale) -> void;

    // Sets absolute orientation; input is normalized before storage.
    auto set_rotation(::glm::quat rotation) -> void;

    // Adds a world-space offset to position.
    auto translate_by(::glm::vec3 offset) -> void;

    // Adds an offset expressed in the entity's local axes (forward/right/up).
    auto translate_local(::glm::vec3 offset_local) -> void;

    // Multiplies current scale by a component-wise factor.
    auto scale_by(::glm::vec3 factor) -> void;

    // Applies a world-space delta rotation in radians (Euler xyz), pre-multiplied.
    auto rotate_by(::glm::vec3 radians) -> void;

    // Applies a local-space delta rotation in radians (Euler xyz), post-multiplied.
    auto rotate_local_by(::glm::vec3 radians_local) -> void;

  private:
    bool active_ = true;
    std::string name_;
    std::uint64_t mesh_handle_;
    Transform transform_;
}; // class Entity

} // namespace pong
