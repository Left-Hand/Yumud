#pragma once

#include <cstdint>

namespace ymd::hal{
namespace GpioTags{
    enum PortSource:uint8_t{
        PA,
        PB,
        PC,
        PD,
        PE,
        PF,
        PG,
        PH,
    };

    enum PinSource:uint8_t{
        _0,
        _1,
        _2,
        _3,
        _4,
        _5,
        _6,
        _7,
        _8,
        _9,
        _10,
        _11,
        _12,
        _13,
        _14,
        _15,
    };
};

template<GpioTags::PortSource port,GpioTags::PinSource pin>
class GpioTag{
};

}
