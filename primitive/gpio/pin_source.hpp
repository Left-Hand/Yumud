#pragma once

#include <cstdint>

namespace ymd::hal{

enum class [[nodiscard]] PortSource:uint8_t{
    PA,
    PB,
    PC,
    PD,
    PE,
    PF
};


enum class [[nodiscard]] PinSource:uint16_t{
    _0 = 1 << 0,
    _1 = 1 << 1,
    _2 = 1 << 2,
    _3 = 1 << 3,
    _4 = 1 << 4,
    _5 = 1 << 5,
    _6 = 1 << 6,
    _7 = 1 << 7,
    _8 = 1 << 8,
    _9 = 1 << 9,
    _10 = 1 << 10,
    _11 = 1 << 11,
    _12 = 1 << 12,
    _13 = 1 << 13,
    _14 = 1 << 14,
    _15 = 1 << 15,
};


template<PortSource _PORT, PinSource _PIN>
struct PinTag{
    static constexpr PortSource PORT = _PORT;
    static constexpr PinSource PIN = _PIN;
};

}