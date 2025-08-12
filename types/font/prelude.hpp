#pragma once

#include "types/vectors/vector2.hpp"

namespace ymd{

enum class TextAlignment:uint8_t{
    Left,
    Center,
    Right,
};

enum class TextBaseline:uint8_t{
    Top,
    Bottom,
    Middle,
    Alphabetic,
};

struct CornerRadii final{
    Vector2<uint8_t> top_left;
    Vector2<uint8_t> top_right;
    Vector2<uint8_t> bottom_right;
    Vector2<uint8_t> bottom_left;
};

struct RoundedRectangle final{
    Rect2i rectangle;
    CornerRadii corners;
};

}