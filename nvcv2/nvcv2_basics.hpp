#pragma once

#include "sys/core/platform.h"
#include "sys/debug/debug_inc.h"

#include "types/color/color_t.hpp"
#include "types/image/Image.hpp"
#include "types/image/PackedImage.hpp"
#include "types/rect2/rect2_t.hpp"

#include "thirdparty/sstl/include/sstl/vector.h" 
#include "thirdparty/sstl/include/sstl/function.h" 


#include <vector>
#include <map>
#include <list>
#include <unordered_set>

using namespace ymd;

namespace ymd::nvcv2{
    using Vector2 = ImageBasics::Vector2;
    using Vector2i = ImageBasics::Vector2i;


    enum class Direction:uint8_t{
        R, UR, U, UL, L, DL, D, DR
    };


    using Histogram = std::array<int, 256>;
}