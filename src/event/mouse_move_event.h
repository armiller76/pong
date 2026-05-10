#pragma once

#include <format>
#include <string>

namespace pong
{

class MouseMoveEvent
{
  public:
    constexpr MouseMoveEvent(const float delta_x, const float delta_y);

    constexpr auto delta_x() const -> float;

    constexpr auto delta_y() const -> float;

    constexpr auto to_string() const -> std::string;

  private:
    float delta_x_;
    float delta_y_;
};

constexpr MouseMoveEvent::MouseMoveEvent(const float delta_x, const float delta_y)
    : delta_x_{delta_x}
    , delta_y_{delta_y}
{
}

constexpr auto MouseMoveEvent::delta_x() const -> float
{
    return delta_x_;
}

constexpr auto MouseMoveEvent::delta_y() const -> float
{
    return delta_y_;
}

constexpr auto MouseMoveEvent::to_string() const -> std::string
{
    return std::format("MouseMoveEvent x:{} y:{}", delta_x_, delta_y_);
}

}
