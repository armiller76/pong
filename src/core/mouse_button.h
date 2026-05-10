#pragma once

#include <string>

namespace pong
{

enum class MouseButton
{
    Left,
    Middle,
    Right,
};

enum class MouseButtonState
{
    Up,
    Down,
};

constexpr auto to_string(MouseButton button) -> std::string
{
    switch (button)
    {
        case MouseButton::Left: return "left";
        case MouseButton::Middle: return "middle";
        case MouseButton::Right: return "right";
        default: return "";
    }
}

constexpr auto to_string(MouseButtonState state) -> std::string
{
    switch (state)
    {
        case MouseButtonState::Down: return "down";
        case MouseButtonState::Up: return "up";
        default: return "";
    }
}
} // namespace pong
