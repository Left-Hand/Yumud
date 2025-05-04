#pragma once

#include "core/utils/Option.hpp"
#include "types/vector2/Vector2.hpp"

namespace ymd::drivers{

class MatrixKeyEvent{
public:
    constexpr Option<uint8_t> row() const {
        return row_;
    }
    constexpr Option<uint8_t> col() const {
        return col_;
    }

    constexpr bool is_pressed() const { return is_pressed_; }
    constexpr MatrixKeyEvent(Option<uint8_t> row, Option<uint8_t> col, bool is_pressed = true):
        row_(row),
        col_(col),
        is_pressed_(is_pressed)
    {;}
private:

    Option<uint8_t> row_;
    Option<uint8_t> col_;
    bool is_pressed_;
};

class CharKeyEvent{
public:
    constexpr Option<char> key() const {
        return key_;
    }

    constexpr CharKeyEvent(Option<char> key):
        key_(key)
    {;}
private:

    Option<char> key_;
};

class MouseEvent{
public:
    constexpr Option<Vector2i> pos() const {
        return pos_;
    }

    constexpr MouseEvent(Option<Vector2i> pos):
        pos_(pos)
    {;}
private:

    Option<Vector2i> pos_;
};

}
