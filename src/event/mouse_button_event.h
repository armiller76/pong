#pragma once

#include <format>
#include <string>

#include "core/mouse_button.h"
#include "utils/formatter.h" // IWYU pragma: keep

namespace pong
{

class MouseButtonEvent
{
  public:
    constexpr MouseButtonEvent(const MouseButton button, const MouseButtonState button_state);

    constexpr auto button() const -> MouseButton;
    constexpr auto button_state() const -> MouseButtonState;

    constexpr auto to_string() const -> std::string;

  private:
    MouseButton button_;
    MouseButtonState button_state_;
};

constexpr MouseButtonEvent::MouseButtonEvent(MouseButton button, MouseButtonState button_state)
    : button_{button}
    , button_state_{button_state}
{
}

constexpr auto MouseButtonEvent::button() const -> MouseButton
{
    return button_;
}

constexpr auto MouseButtonEvent::button_state() const -> MouseButtonState
{
    return button_state_;
}

constexpr auto MouseButtonEvent::to_string() const -> std::string
{
    return std::format("MouseButtonEvent {}-{}", button_, button_state_);
}

} // namespace pong
