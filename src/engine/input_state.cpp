#include "engine/input_state.h"

#include <queue>
#include <type_traits>

#include <glm/vec3.hpp>

#include "core/key.h"
#include "event/key_event.h"
#include "graphics/camera.h"
#include "math/utils.h"
#include "utils/error.h"
#include "utils/log.h"

namespace pong
{

InputState::InputState()
    : events_{}
    , keyboard_state_{}
    , dirty_keys_{}
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
                        auto &key = keyboard_state_[index.value()];
                        switch (arg.position())
                        {
                            case KeyPosition::Down:
                            {
                                key.is_down_this_frame = true;
                                if (!key.was_down_last_frame)
                                {
                                    key.was_pressed_this_frame = true;
                                }
                            }
                            break;
                            case KeyPosition::Up:
                            {
                                key.is_down_this_frame = false;
                                key.was_released_this_frame = true;
                            }
                            break;
                        }
                    }
                    else
                    {
                        // key ignored
                    }
                } // else if constexpr (other EventTypes)
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
        auto &key_state = keyboard_state_[index.value()];
        key_state.was_down_last_frame = key_state.is_down_this_frame;
        key_state.was_pressed_this_frame = false;
        key_state.was_released_this_frame = false;
    }
    dirty_keys_.clear();
}

auto InputState::move_direction(const Camera &camera, const float speed) -> ::glm::vec3
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

} // namespace pong
