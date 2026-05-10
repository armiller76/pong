#pragma once

#include <variant>

#include "event/key_event.h"

namespace pong
{

using Event = std::variant<KeyEvent>;

}
