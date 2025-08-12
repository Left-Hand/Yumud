#pragma once

#include "core/platform.hpp"
#include "core/debug/debug.hpp"

#include "types/colors/color/color.hpp"
#include "types/image/image.hpp"
#include "types/image/packed_image.hpp"
#include "types/regions/rect2.hpp"

#include "thirdparty/sstl/include/sstl/vector.h" 
#include "thirdparty/sstl/include/sstl/function.h" 


#include <vector>
#include <map>
#include <list>
#include <unordered_set>

using namespace ymd;

namespace ymd::nvcv2{
    enum class Direction:uint8_t{
        R, UR, U, UL, L, DL, D, DR
    };


    using Histogram = std::array<int, 256>;
}