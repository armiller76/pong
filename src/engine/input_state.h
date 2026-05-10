#pragma once

#include <array>
#include <queue>
#include <unordered_set>

#include <glm/vec3.hpp>

#include "core/key.h"
#include "event/event.h"

namespace pong
{

class Camera;

class InputState
{
  public:
    struct ButtonState
    {
        bool is_down_this_frame;
        bool was_pressed_this_frame;
        bool was_released_this_frame;
        bool was_down_last_frame;
    };

    struct MouseState
    {
        ButtonState l_button_state;
        ButtonState m_button_state;
        ButtonState r_button_state;
        float frame_delta_x;
        float frame_delta_y;
        float frame_delta_wheel;
    };

    InputState();

    auto events(this auto &&self) -> auto &&;

    auto keyboard_state(this auto &&self) -> auto &&;

    auto mouse_state(this auto &&self) -> auto &&;

    auto dirty_keys(this auto &&self) -> auto &&;

    auto process_events() -> void;

    auto advance_frame() -> void;

    auto move_direction(const Camera &camera, const float speed) -> ::glm::vec3;

  private:
    std::queue<Event> events_;
    std::array<ButtonState, all_keys.size()> keyboard_state_;
    std::unordered_set<Key> dirty_keys_;
    MouseState mouse_state_;

    auto reset_button_state_(ButtonState &state) -> void;

    auto update_button_state_(ButtonState &state, bool down) -> void;

}; // class InputState

auto InputState::events(this auto &&self) -> auto &&
{
    return self.events_;
}

auto InputState::keyboard_state(this auto &&self) -> auto &&
{
    return self.keyboard_state_;
}

auto InputState::mouse_state(this auto &&self) -> auto &&
{
    return self.mouse_state_;
}

auto InputState::dirty_keys(this auto &&self) -> auto &&
{
    return self.dirty_keys_;
}

} // namespace pong
