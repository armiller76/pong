#pragma once

#include <variant>

#include "event/key_event.h"
#include "event/mouse_button_event.h"
#include "event/mouse_move_event.h"

namespace pong
{

// TODO Add MouseWheelEvent
using Event = std::variant<KeyEvent, MouseMoveEvent, MouseButtonEvent>;

}
