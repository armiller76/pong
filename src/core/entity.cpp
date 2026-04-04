#include "entity.h"

#include <string>
#include <string_view>

#include "graphics/glm_wrapper.h"
#include "graphics/mesh.h"
#include "math/transform.h"
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

auto Entity::set_position(::glm::vec3 position) -> void
{
    // TODO GIGO
    transform_.position = position;
}

auto Entity::set_scale(::glm::vec3 scale) -> void
{
    // TODO GIGO
    transform_.scale = scale;
}

auto Entity::set_rotation(::glm::quat rotation) -> void
{
    // TODO GIGO
    transform_.rotation = rotation;
}

} // namespace pong
