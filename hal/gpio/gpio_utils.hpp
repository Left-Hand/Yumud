#pragma once

#include <cstdint>
#include "core/platform.hpp"

namespace ymd::hal{


enum class PinSource:uint16_t{
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

class PinMask{
public:
    explicit constexpr PinMask(const uint16_t raw):
        raw_(raw){;}

    explicit constexpr PinMask(const PinSource source):
        raw_(std::bit_cast<uint16_t>(raw_)){;}

    static constexpr PinMask from_u16(const uint16_t raw){
        return PinMask(raw);
    }

    constexpr uint16_t to_u16() const {return raw_;}
    constexpr PinSource to_source() const {
        return std::bit_cast<PinSource>(raw_);}

    constexpr PinMask operator | (const PinMask other) const {
        return PinMask(raw_ | other.raw_);
    }

    constexpr PinMask operator & (const PinMask other) const {
        return PinMask(raw_ & other.raw_);
    }

    constexpr PinMask operator ~() const {
        return PinMask(~raw_);
    }
private:
    uint16_t raw_;
};

enum class PortSource:uint8_t{
    PA,
    PB,
    PC,
    PD,
    PE,
    PF
};

enum class PinName:uint8_t{
    #define PINNAME_CREATE_TEMPLATE(x, n)\
    P##x##0 = n | 0b00000, P##x##1, P##x##2, P##x##3, P##x##4, P##x##5, P##x##6, P##x##7,\
    P##x##8, P##x##9, P##x##10, P##x##11, P##x##12, P##x##13, P##x##14, P##x##15,\
    P##x##16, P##x##17, P##x##18, P##x##19, P##x##20, P##x##21, P##x##22, P##x##23,\
    P##x##24, P##x##25, P##x##26, P##x##27, P##x##28, P##x##29, P##x##30, P##x##31\

    #ifdef ENABLE_GPIOA
    PINNAME_CREATE_TEMPLATE(A, 0)
    #endif

    #ifdef ENABLE_GPIOB
    ,
    PINNAME_CREATE_TEMPLATE(B, 1)
    #endif

    #ifdef ENABLE_GPIOC
    ,
    PINNAME_CREATE_TEMPLATE(C, 2)
    #endif

    #ifdef ENABLE_GPIOD
    ,
    PINNAME_CREATE_TEMPLATE(D, 3)
    #endif

    #ifdef ENABLE_GPIOE
    ,
    PINNAME_CREATE_TEMPLATE(E, 4)
    #endif

    #ifdef ENABLE_GPIOF
    ,
    PINNAME_CREATE_TEMPLATE(F, 5)
    #endif

    #ifdef ENABLE_GPIOG
    ,
    PINNAME_CREATE_TEMPLATE(G, 6)
    #endif
};

class GpioMode{
public:
    enum class Kind:uint8_t{
        InAnalog = 0b0000,
        InFloating = 0b0100,
        InPullUP = 0b1000,
        InPullDN = 0b1100,
        OutPP = 0b0011,
        OutOD = 0b0111,
        OutAfPP = 0b1011,
        OutAfOD = 0b1111
    };
    using enum Kind;

    constexpr GpioMode(const Kind kind):kind_(kind){;}

    constexpr bool operator ==(const Kind kind) const 
        {return kind_ == kind;}
    constexpr bool operator ==(const GpioMode other) const
        {return kind_ == other.kind_;}
    constexpr bool is_in_mode() const {
        return (kind_ == InAnalog) 
        || (kind_ == InFloating) 
        || (kind_ == InPullUP) 
        || (kind_ == InPullDN);
    }

    constexpr bool is_out_mode() const {
        return (kind_ == OutPP)
        || (kind_ == OutOD)
        || (kind_ == OutAfPP)
        || (kind_ == OutAfOD);
    }

    constexpr bool is_outpp_mode() const {
        return (kind_ == OutPP)
        || (kind_ == OutAfPP);
    }

    constexpr bool is_outod_mode() const {
        return (kind_ == OutOD)
        || (kind_ == OutAfOD);
    }

    explicit operator uint8_t() const {
        return std::bit_cast<uint8_t>(kind_);
    }
private:
    Kind kind_;
};

}