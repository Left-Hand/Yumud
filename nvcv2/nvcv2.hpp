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


    enum class Direction:uint8_t{
        R, UR, U, UL, L, DL, D, DR
    };


    using Histogram = std::array<int, 256>;
}


__fast_inline OutputStream & operator<<(OutputStream & os, const NVCV2::TB tb){
    using namespace NVCV2;
    switch(tb){
        case TB::TOP: return os << 'T';break;
        case TB::BOTTOM: return os << 'B';break;
        default: return os << '?';break;
    };
}


__fast_inline OutputStream & operator<<(OutputStream & os, const NVCV2::LR lr){
    using namespace NVCV2;
    switch(lr){
        case LR::LEFT: return os << 'L';break;
        case LR::RIGHT: return os << 'R';break;
        default: return os << '?';break;
    };
}

#endif