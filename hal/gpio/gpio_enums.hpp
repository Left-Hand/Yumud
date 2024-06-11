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


enum class PinName:uint8_t{
    #define PINNAME_CREATE_TEMPLATE(x, n)\
    P##x##0 = n | 0b00000, P##x##1, P##x##2, P##x##3, P##x##4, P##x##5, P##x##6, P##x##7,\
    P##x##8, P##x##9, P##x##10, P##x##11, P##x##12, P##x##13, P##x##14, P##x##15,\
    P##x##16, P##x##17, P##x##18, P##x##19, P##x##20, P##x##21, P##x##22, P##x##23,\
    P##x##24, P##x##25, P##x##26, P##x##27, P##x##28, P##x##29, P##x##30, P##x##31\

    #ifdef HAVE_GPIOA
    PINNAME_CREATE_TEMPLATE(A, 0)
    #endif

    #ifdef HAVE_GPIOB
    ,
    PINNAME_CREATE_TEMPLATE(B, 1)
    #endif

    #ifdef HAVE_GPIOC
    ,
    PINNAME_CREATE_TEMPLATE(C, 2)
    #endif

    #ifdef HAVE_GPIOD
    ,
    PINNAME_CREATE_TEMPLATE(D, 3)
    #endif

    #ifdef HAVE_GPIOE
    ,
    PINNAME_CREATE_TEMPLATE(E, 4)
    #endif

    #ifdef HAVE_GPIOF
    ,
    PINNAME_CREATE_TEMPLATE(F, 5)
    #endif

    #ifdef HAVE_GPIOG
    ,
    PINNAME_CREATE_TEMPLATE(G, 6)
    #endif
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

namespace PinModeUtils{
    constexpr bool isIn(const PinMode & pinmode){
        return pinmode == PinMode::InAnalog || pinmode == PinMode::InFloating || pinmode == PinMode::InPullUP || pinmode == PinMode::InPullDN;
    }

    constexpr bool isOut(const PinMode & pinmode){
        return pinmode == PinMode::OutPP || pinmode == PinMode::OutOD || pinmode == PinMode::OutAfPP || pinmode == PinMode::OutAfOD;
    }

    constexpr bool isPP(const PinMode & pinmode){
        return pinmode == PinMode::OutPP || pinmode == PinMode::OutAfPP;
    }

    constexpr bool isOD(const PinMode & pinmode){
        return pinmode == PinMode::OutOD || pinmode == PinMode::OutAfOD;
    }
}

#endif