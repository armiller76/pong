#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "core/resource_handles.h"
#include "graphics/model.h"
#include "math/transform.h"

namespace pong
{

class Entity
{
  public:
    Entity() = default;
    Entity(std::string_view name, Model model, Transform transform);
    ~Entity() = default;

    Entity(const Entity &) = delete;
    auto operator=(const Entity &) -> Entity & = delete;
    Entity(Entity &&) noexcept = default;
    auto operator=(Entity &&) noexcept -> Entity & = default;

    auto set_active(bool active) -> void;
    auto is_active() const -> bool;

    auto name() const -> std::string_view;

    auto model() -> std::optional<Model> &;
    auto model() const -> const std::optional<Model> &;

    auto children() const -> const std::vector<EntityIndex> &;
    auto child_count() const -> std::size_t;

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

    // TODO implement
    // Adds an offset expressed in the entity's local axes (forward/right/up).
    //    auto translate_local(::glm::vec3 offset_local) -> void;

    // Multiplies current scale by a component-wise factor.
    auto scale_by(::glm::vec3 factor) -> void;

    // Applies a world-space delta rotation in radians (Euler xyz), pre-multiplied.
    auto rotate_by(::glm::vec3 radians) -> void;

    // TODO implement
    // Applies a local-space delta rotation in radians (Euler xyz), post-multiplied.
    //   auto rotate_local_by(::glm::vec3 radians_local) -> void;

    auto set_name(std::string_view name) -> void;
    auto set_transform(Transform transform) -> void;
    auto add_child(EntityIndex index) -> void;

  private:
    bool active_ = true;
    std::string name_;
    std::optional<Model> model_;
    Transform transform_;
    std::vector<EntityIndex> children_;
}; // class Entity

} // namespace pong
