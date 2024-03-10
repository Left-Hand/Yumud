#ifndef __GPIO_ENUMS_H__

#define __GPIO_ENUMS_H__

enum class Pin:uint16_t{
    None,
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

enum class PinMode:uint8_t{
    InAnalog = 0b0000,
    InFloating = 0b0100,
    InPullUP = 0b1000,
    InPullDN = 0b1100,
    OutPP = 0b0011,
    OutOD = 0b0111,
    OutAfPP = 0b1011,
    OutAfOD = 0b1111
};


#endif