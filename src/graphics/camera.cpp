#include "camera.h"

#include "glm_wrapper.h"

namespace pong
{

Camera::Camera(::glm::vec3 eye, ::glm::vec3 look_at, ::glm::vec3 up)
    : eye_{eye}
    , center_{look_at}
    , up_{up}
{
    // TODO GIGO
}

auto Camera::translate(::glm::vec3 offset) -> void
{
    // TODO GIGO
    eye_ += offset;
    center_ += offset;
}

auto Camera::set_position(::glm::vec3 position) -> void
{
    // TODO GIGO
    auto offset = position - eye_;
    eye_ = position;
    center_ += offset;
}

auto Camera::set_view_target(::glm::vec3 target) -> void
{
    // TODO GIGO
    center_ = target;
}

auto Camera::get_view_matrix() const -> ::glm::mat4
{
    return ::glm::lookAt(eye_, center_, up_);
}

} // namespace pong
