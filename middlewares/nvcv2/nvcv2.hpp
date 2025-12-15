#pragma once

#include "core/platform.hpp"
#include "core/debug/debug.hpp"

#include "primitive/colors/color/color.hpp"
#include "primitive/image/image.hpp"
#include "primitive/image/packed_image.hpp"
#include "algebra/regions/rect2.hpp"


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