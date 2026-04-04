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
    Entity(std::string_view name, Mesh &mesh, Transform &transform);

    auto set_active(bool active) -> void;
    auto is_active() const -> bool;

    auto name() const -> std::string_view;

    auto mesh() const -> const Mesh &;

    auto transform(this auto &&self) -> auto &&;

  private:
    bool active_ = true;
    std::string name_;
    Mesh &mesh_;
    Transform transform_;
};

}
