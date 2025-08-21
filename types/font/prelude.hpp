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



// struct RoundedRectangle final{
//     Rect2i rectangle;
//     CornerRadii corners;
// };

}