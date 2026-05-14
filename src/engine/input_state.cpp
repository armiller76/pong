#include "engine/input_state.h"

#include <queue>
#include <type_traits>

#include <glm/vec3.hpp>

#include "core/key.h"
#include "core/mouse_button.h"
#include "event/key_event.h"
#include "event/mouse_move_event.h"
#include "graphics/free_look_camera.h"
#include "math/utils.h"
#include "utils/error.h"
#include "utils/log.h"

namespace pong
{

InputState::InputState()
    : events_{}
    , keyboard_state_{}
    , dirty_keys_{}
    , mouse_state_{}
{
    arm::log::debug("InputState constructor");
}

auto InputState::process_events() -> void
{
    while (!events_.empty())
    {
        auto &e = events_.front();
        events_.pop();

        std::visit(
            [&](auto &&arg)
            {
                using InType = std::decay_t<decltype(arg)>;
                if constexpr (std::same_as<InType, KeyEvent>)
                {
                    const auto index = get_key_index(arg.key());
                    if (index.has_value())
                    {
                        update_button_state_(
                            keyboard_state_[index.value()], arg.position() == KeyPosition::Down ? true : false);
                    }
                    else
                    {
                        // key ignored
                    }
                }
                else if constexpr (std::same_as<InType, MouseMoveEvent>)
                {
                    mouse_state_.frame_delta_x += arg.delta_x();
                    mouse_state_.frame_delta_y += arg.delta_y();
                }
                else if constexpr (std::same_as<InType, MouseButtonEvent>)
                {
                    switch (arg.button())
                    {
                        case MouseButton::Left:
                        {
                            update_button_state_(
                                mouse_state_.l_button_state,
                                arg.button_state() == MouseButtonState::Down ? true : false);
                        }
                        break;
                        case MouseButton::Middle:
                        {
                            update_button_state_(
                                mouse_state_.m_button_state,
                                arg.button_state() == MouseButtonState::Down ? true : false);
                        }
                        break;
                        case MouseButton::Right:
                        {
                            update_button_state_(
                                mouse_state_.r_button_state,
                                arg.button_state() == MouseButtonState::Down ? true : false);
                        }
                        break;
                    }
                }
            },
            e);
    }
}

auto InputState::advance_frame() -> void
{
    for (const auto k : dirty_keys_)
    {
        const auto index = get_key_index(k);
        arm::ensure(index.has_value(), "dirty_keys invariant is broken");
        reset_button_state_(keyboard_state_[index.value()]);
    }
    dirty_keys_.clear();

    reset_button_state_(mouse_state_.l_button_state);
    reset_button_state_(mouse_state_.m_button_state);
    reset_button_state_(mouse_state_.r_button_state);
    mouse_state_.frame_delta_wheel = 0.0f;
    mouse_state_.frame_delta_x = 0.0f;
    mouse_state_.frame_delta_y = 0.0f;
}

auto InputState::move_direction(const FreeLookCamera &camera, const float speed) -> ::glm::vec3
{
    auto result = ::glm::vec3{0.0f};

    if (keyboard_state_[get_key_index(Key::W).value()].is_down_this_frame)
    {
        result += camera.get_forward_direction();
    }
    if (keyboard_state_[get_key_index(Key::S).value()].is_down_this_frame)
    {
        result -= camera.get_forward_direction();
    }
    if (keyboard_state_[get_key_index(Key::A).value()].is_down_this_frame)
    {
        result -= camera.get_right_direction();
    }
    if (keyboard_state_[get_key_index(Key::D).value()].is_down_this_frame)
    {
        result += camera.get_right_direction();
    }

    if (::glm::length(result) < g_epsilon)
    {
        return ::glm::vec3{0.0f, 0.0f, 0.0f};
    }

    return ::glm::normalize(result) * speed;
}

auto InputState::reset_button_state_(ButtonState &state) -> void
{
    state.was_down_last_frame = state.is_down_this_frame;
    state.was_pressed_this_frame = false;
    state.was_released_this_frame = false;
}

auto InputState::update_button_state_(ButtonState &state, bool down) -> void
{
    if (down)
    {
        state.is_down_this_frame = true;
        if (state.was_down_last_frame)
        {
            state.was_pressed_this_frame = true;
        }
    }
    else
    {
        state.is_down_this_frame = false;
        state.was_released_this_frame = true;
    }
}

} // namespace pong
