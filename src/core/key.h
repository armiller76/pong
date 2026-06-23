#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

#include "utils/exception.h"

namespace pong
{

// ADD NEW KEYS IN THREE PLACES!
// enum class Key
// all-keys array
// to_string(key)

enum class Key : std::uint8_t
{
    Backspace = 0x08,
    Enter = 0x0D,
    Space = 0x20,

    N0 = 0x30,
    N1 = 0x31,
    N2 = 0x32,
    N3 = 0x33,
    N4 = 0x34,
    N5 = 0x35,
    N6 = 0x36,
    N7 = 0x37,
    N8 = 0x38,
    N9 = 0x39,

    A = 0x41,
    B = 0x42,
    C = 0x43,
    D = 0x44,
    E = 0x45,
    F = 0x46,
    G = 0x47,
    H = 0x48,
    I = 0x49,
    J = 0x4A,
    K = 0x4B,
    L = 0x4C,
    M = 0x4D,
    N = 0x4E,
    O = 0x4F,
    P = 0x50,
    Q = 0x51,
    R = 0x52,
    S = 0x53,
    T = 0x54,
    U = 0x55,
    V = 0x56,
    W = 0x57,
    X = 0x58,
    Y = 0x59,
    Z = 0x5A,

    Escape = 0x1B,

    LShift = 0xA0,
    RShift = 0xA1,
    LControl = 0xA2,
    RControl = 0xA3,
    LAlt = 0xA4,
    RAlt = 0xA5,

    F1 = 0x70,
    F2 = 0x71,
    F3 = 0x72,
    F4 = 0x73,
    F5 = 0x74,
    F6 = 0x75,
    F7 = 0x76,
    F8 = 0x77,
    F9 = 0x78,
    F10 = 0x79,
    F11 = 0x7A,
    F12 = 0x7B,

    LWindows = 0x5B,
    RWindows = 0x5C,
    Context = 0x5D,

    LArrow = 0x25,
    UArrow = 0x26,
    RArrow = 0x27,
    DArrow = 0x28,

    Backtick = 0xC0,
    Minus = 0xBD,
    Equals = 0xBB,
}; // enum class Key

inline constexpr auto all_keys = std::to_array({
    Key::Backspace, Key::Enter,    Key::Space,    Key::N0,       Key::N1,      Key::N2,     Key::N3,     Key::N4,
    Key::N5,        Key::N6,       Key::N7,       Key::N8,       Key::N9,      Key::A,      Key::B,      Key::C,
    Key::D,         Key::E,        Key::F,        Key::G,        Key::H,       Key::I,      Key::J,      Key::K,
    Key::L,         Key::M,        Key::N,        Key::O,        Key::P,       Key::Q,      Key::R,      Key::S,
    Key::T,         Key::U,        Key::V,        Key::W,        Key::X,       Key::Y,      Key::Z,      Key::Escape,
    Key::LShift,    Key::RShift,   Key::LControl, Key::RControl, Key::LAlt,    Key::RAlt,   Key::F1,     Key::F2,
    Key::F3,        Key::F4,       Key::F5,       Key::F6,       Key::F7,      Key::F8,     Key::F9,     Key::F10,
    Key::F11,       Key::F12,      Key::LWindows, Key::RWindows, Key::Context, Key::LArrow, Key::UArrow, Key::RArrow,
    Key::DArrow,    Key::Backtick, Key::Minus,    Key::Equals,
});

enum class KeyPosition : std::uint8_t
{
    Up,
    Down,
}; // enum class KeyPosition

constexpr auto to_string(const Key &key) -> std::string
{
    switch (key)
    {
        using enum Key;
        case Backspace: return "Bksp";
        case Enter: return "Enter";
        case Space: return "Space";
        case N0: return "Num0";
        case N1: return "Num1";
        case N2: return "Num2";
        case N3: return "Num3";
        case N4: return "Num4";
        case N5: return "Num5";
        case N6: return "Num6";
        case N7: return "Num7";
        case N8: return "Num8";
        case N9: return "Num9";
        case A: return "A";
        case B: return "B";
        case C: return "C";
        case D: return "D";
        case E: return "E";
        case F: return "F";
        case G: return "G";
        case H: return "H";
        case I: return "I";
        case J: return "J";
        case K: return "K";
        case L: return "L";
        case M: return "M";
        case N: return "N";
        case O: return "O";
        case P: return "P";
        case Q: return "Q";
        case R: return "R";
        case S: return "S";
        case T: return "T";
        case U: return "U";
        case V: return "V";
        case W: return "W";
        case X: return "X";
        case Y: return "Y";
        case Z: return "Z";
        case Escape: return "Esc";
        case LShift: return "LShft";
        case RShift: return "RShft";
        case LControl: return "LCtrl";
        case RControl: return "RCtrl";
        case LAlt: return "LAlt";
        case RAlt: return "RAlt";
        case F1: return "F1";
        case F2: return "F2";
        case F3: return "F3";
        case F4: return "F4";
        case F5: return "F5";
        case F6: return "F6";
        case F7: return "F7";
        case F8: return "F8";
        case F9: return "F9";
        case F10: return "F10";
        case F11: return "F11";
        case F12: return "F12";
        case LWindows: return "LWindows";
        case RWindows: return "RWindows";
        case Context: return "ContextMenu";
        case LArrow: return "LeftArrow";
        case UArrow: return "UpArrow";
        case RArrow: return "RightArrow";
        case DArrow: return "DownArrow";
        case Backtick: return "`";
        case Minus: return "-";
        case Equals: return "=";
        default: return "<unknown>";
    }
}

constexpr auto get_key_index(Key key) -> std::size_t
{
    const auto result = std::ranges::find(all_keys, key);
    if (result != all_keys.end())
    {
        return result - all_keys.begin();
    }
    throw arm::Exception("Unknown key");
}

constexpr auto to_string(const KeyPosition &position) -> std::string
{
    switch (position)
    {
        case KeyPosition::Down: return "Down";
        case KeyPosition::Up: return "Up";
        default: return "<unknown>";
    }
}

} // namespace pong
