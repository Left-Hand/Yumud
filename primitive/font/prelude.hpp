#pragma once

#include "algebra/vectors/vec2.hpp"

namespace ymd{

enum class [[nodiscard]] TextAlignment:uint8_t{
    Left,
    Center,
    Right,
};

enum class [[nodiscard]] TextBaseline:uint8_t{
    Top,
    Bottom,
    Middle,
    Alphabetic,
};

}