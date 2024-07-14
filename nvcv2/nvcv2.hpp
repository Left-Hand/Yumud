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

    enum CornerType{
        ACORNER = -1,
        ALL = 0,
        VCORNER = 1
    };


    enum class Direction:uint8_t{
        R, UR, U, UL, L, DL, D, DR
    };

    using Coast = sstl::vector<Vector2i, 80>;
    using Boundry = std::map<int, int>;
    using Pile = std::pair<int, Rangei>;
    using Piles = std::map<int, Rangei>;
    using Point = Vector2i; 
    using Points = sstl::vector<Point, 64>;
    using Ranges = sstl::vector<Rangei, 64>;

    using Segment = std::pair<Point, Point>;
    using Corner = std::pair<CornerType,Vector2i>;
    using Corners = sstl::vector<Corner, 8>;
    using Coasts = sstl::vector<Coast, 4>;

    using Histogram = std::array<int, 256>;
}

#endif