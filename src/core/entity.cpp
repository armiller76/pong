#include "entity.h"

#include <string>
#include <string_view>

#include "graphics/glm_wrapper.h"
#include "graphics/mesh.h"
#include "math/transform.h"
#include "math/utils.h"
#include "utils/error.h"

namespace pong
{

Entity::Entity(std::string_view name, std::uint64_t mesh_handle, Transform transform)
    : name_{name}
    , mesh_handle_{mesh_handle}
    , transform_{std::move(transform)}
{
}

auto Entity::set_active(bool active) -> void
{
    if (active_ == active)
    {
        arm::log::warn("trying to set entity active state same as previous state: {}", name_);
        return;
    }
    active_ = active;
}

auto Entity::is_active() const -> bool
{
    return active_;
}

auto Entity::name() const -> std::string_view
{
    return name_;
}

auto Entity::mesh_handle() const -> std::uint64_t
{
    return mesh_handle_;
}

auto Entity::transform() -> Transform &
{
    return transform_;
}

auto Entity::transform() const -> const Transform &
{
    return transform_;
}

// Position and translation are in world space unless the method name says local.
// Rotation quaternions are stored normalized.

auto Entity::set_position(::glm::vec3 position) -> void
// Sets absolute world position.
{
    if (!all_finite(position))
    {
        arm::log::warn("entity: set_position position has infinite component");
        return;
    }

    transform_.position = position;
}

auto Entity::set_scale(::glm::vec3 scale) -> void
// Sets absolute scale (component-wise).
{
    if (!is_scale_safe(scale))
    {
        arm::log::warn("entity: set_scale scale has infinite component or component < epsilon");
        return;
    }

    transform_.scale = scale;
}

auto Entity::set_rotation(::glm::quat rotation) -> void
// Sets absolute orientation; input is normalized before storage.
{
    transform_.rotation = normalize_safe(rotation);
}

auto Entity::translate_by(::glm::vec3 offset) -> void
// Adds a world-space offset to position.
{
    if (!all_finite(offset))
    {
        arm::log::warn("entity: translate_by offset has infinite component or component < epsilon");
        return;
    }

    transform_.position += offset;
}

auto Entity::translate_local(::glm::vec3 offset_local) -> void
// Adds an offset expressed in the entity's local axes (forward/right/up).
{
    if (!all_finite(offset_local))
    {
        arm::log::warn("entity: translate_local_by offset_local has infinite component or component < epsilon");
        return;
    }

    arm::not_implemented();
}

auto Entity::scale_by(::glm::vec3 factor) -> void
// Multiplies current scale by a component-wise factor.
{
    if (!is_scale_safe(factor))
    {
        arm::log::warn("entity: scale_by unsafe factor");
    }

    transform_.scale *= factor;
}

auto Entity::rotate_by(::glm::vec3 radians) -> void
// Applies a world-space delta rotation in radians (Euler xyz), pre-multiplied.
{
    if (!all_finite(radians))
    {
        arm::log::warn("entity: rotate_by radians infinite");
        return;
    }

    transform_.rotation = normalize_safe(::glm::quat(radians) * transform_.rotation);
}

auto Entity::rotate_local_by(::glm::vec3 radians_local) -> void
// Applies a local-space delta rotation in radians (Euler xyz), post-multiplied.
{
    if (!all_finite(radians_local))
    {
        arm::log::warn("entity: rotate_local_by radians_local infinite");
        return;
    }

    arm::not_implemented();
}

} // namespace pong
