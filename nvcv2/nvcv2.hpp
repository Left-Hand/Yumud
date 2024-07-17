#ifndef __NVCV2_HPP__

#define __NVCV2_HPP__

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

    enum class CornerType:uint8_t{
        NONE,
        AC,
        VC,
        ALL
    };



    enum class Direction:uint8_t{
        R, UR, U, UL, L, DL, D, DR
    };


    using Histogram = std::array<int, 256>;
}

#endif