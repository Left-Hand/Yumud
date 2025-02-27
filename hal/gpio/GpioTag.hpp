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

namespace PeriphTags{
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

    enum ClkSource:uint8_t{
        AHB,
        APB1,
        APB2,
    };
}


template<GpioTags::PortSource port,GpioTags::PinSource pin>
struct __GpioTag{
    static constexpr auto port_source = port;
    static constexpr auto pin_source = pin;
};

template<GpioTags::PortSource port,GpioTags::PinSource pin>
struct GpioTag:public __GpioTag<port, pin>{
};

}
