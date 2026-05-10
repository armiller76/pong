#pragma once

#include <format>
#include <string>

#include "core/key.h"
#include "utils/formatter.h" // IWYU pragma: keep

namespace pong
{

class KeyEvent
{
  public:
    constexpr KeyEvent(Key key, KeyPosition position)
        : key_{key}
        , position_{position}
    {
    }

    constexpr auto key() const -> Key
    {
        return key_;
    }

    constexpr auto position() const -> KeyPosition
    {
        return position_;
    };

    constexpr auto operator==(const KeyEvent &) const -> bool = default;

    constexpr auto to_string() const -> std::string
    {
        return std::format("KeyEvent {}_{}", key_, position_);
    }

  private:
    Key key_;
    KeyPosition position_;
};

} // namespace pong
