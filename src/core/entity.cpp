#include "entity.h"

#include <string>
#include <string_view>

#include "graphics/glm_wrapper.h"
#include "graphics/mesh.h"
#include "math/transform.h"
#include "utils/error.h"

namespace pong
{

Entity::Entity(std::string_view name, Mesh &mesh, Transform &transform)
    : name_{name}
    , mesh_{mesh}
    , transform_{transform}
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

auto Entity::mesh() const -> const Mesh &
{
    return mesh_;
}

auto Entity::transform(this auto &&self) -> auto &&
{
    return self.transform_;
}

}
