#pragma once

#include "../sys/core/system.hpp"
#include "../sys/debug/debug_inc.h"

#include "../types/color/color_t.hpp"
#include "../types/image/image.hpp"
#include "../types/image/packed_image.hpp"
#include "../types/image/painter.hpp"
#include "../types/rect2/rect2_t.hpp"

#include "../thirdparty/sstl/include/sstl/vector.h" 
#include "../thirdparty/sstl/include/sstl/function.h" 


#include <vector>
#include <map>
#include <list>
#include <unordered_set>

namespace NVCV2{
    enum TB{
        TOP = false,
        BOTTOM = true
    };

    enum LR{
        LEFT = false,
        RIGHT = true
    };


    enum class Direction:uint8_t{
        R, UR, U, UL, L, DL, D, DR
    };


    using Histogram = std::array<int, 256>;
}


OutputStream & operator<<(OutputStream & os, const NVCV2::TB tb);

OutputStream & operator<<(OutputStream & os, const NVCV2::LR lr);