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

enum class ClkSource:uint8_t{
    AHB,
    APB1,
    APB2,
};
template<GpioTags::PortSource PORT_SOURCE,GpioTags::PinSource PIN_SOURCE>
struct __GpioTag{
    static constexpr auto port_source = PORT_SOURCE;
    static constexpr auto pin_source = PIN_SOURCE;
};

template<GpioTags::PortSource PORT_SOURCE,GpioTags::PinSource PIN_SOURCE>
struct GpioTag:public __GpioTag<PORT_SOURCE, PIN_SOURCE>{
};

}
