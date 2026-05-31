#pragma once

#include "mavlink_primitive.hpp"

namespace ymd::mavlink{


struct LoadPercents{
    uint16_t bits;
};

struct VoltageCode{
    //mv
    uint16_t bits;
};

struct CurrentCode{
    //cA
    int16_t bits;

    constexpr bool is_invalid(){
        return bits == -1;
    }
};

template<typename T>
struct Radians{
    T bits;
};


}