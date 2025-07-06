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
    [[nodiscard]] constexpr PinMask():
        raw_(0){;}

    [[nodiscard]] explicit constexpr PinMask(const uint16_t raw):
        raw_(raw){;}

    [[nodiscard]] constexpr PinMask(const PinSource source):
        raw_(std::bit_cast<uint16_t>(raw_)){;}

    [[nodiscard]] static constexpr PinMask from_u16(const uint16_t raw){
        return PinMask(raw);
    }

    [[nodiscard]] static constexpr PinMask from_index(const size_t index){
        return PinMask(uint16_t(1 << index));
    }
    [[nodiscard]] constexpr uint16_t as_u16() const {return raw_;}
    [[nodiscard]] constexpr PinSource as_source() const {
        return std::bit_cast<PinSource>(raw_);}


    [[nodiscard]] constexpr bool test(size_t idx) const {
        return raw_ & (1 << idx);
    }

    [[nodiscard]] constexpr PinMask modify(size_t idx, const BoolLevel level) const {
        if(level == HIGH) return PinMask(raw_ | (1 << idx));
        else return PinMask(raw_ & (~(1 << idx)));
    }

    [[nodiscard]] constexpr PinMask set_bit(size_t idx) const {
        return PinMask(raw_ | (1 << idx));
    }

    [[nodiscard]] constexpr PinMask clr_bit(size_t idx) const {
        return PinMask(raw_ & (~(1 << idx)));
    }

    [[nodiscard]] constexpr PinMask operator | (const PinMask other) const {
        return PinMask(raw_ | other.raw_);
    }

    [[nodiscard]] constexpr PinMask operator & (const PinMask other) const {
        return PinMask(raw_ & other.raw_);
    }

    [[nodiscard]] constexpr PinMask operator ~() const {
        return PinMask(~raw_);
    }

    [[nodiscard]] constexpr bool operator == (const PinMask & other) const{
        return  raw_ == other.raw_;
    }

    [[nodiscard]] explicit constexpr operator bool() const {
        return raw_;
    }

    struct Iterator final{
    public:
        constexpr Iterator(uint16_t mask) : 
            mask_(mask), 
            pos_(next_set_bit(mask_, 0)) {}
        [[nodiscard]] constexpr bool has_next() const {return pos_ < 16;}
        constexpr hal::PinSource next(){
            const uint16_t ret = 1 << pos_;
            pos_ = next_set_bit(mask_, pos_ + 1);
            return std::bit_cast<hal::PinSource>(ret);
        }

        [[nodiscard]] constexpr size_t index() const {
            return pos_;
        }
    private:
        uint16_t mask_;
        size_t pos_;
        
        static constexpr size_t next_set_bit(uint16_t mask, size_t start) {
            for (size_t i = start; i < 16; ++i) {
                if (mask & (1 << i)) return i;
            }
            return 16; // End of mask
        }
    };

    [[nodiscard]] Iterator iter() const {
        return Iterator{raw_};
    }
private:
    uint16_t raw_;
};


// struct PinSource{
//     static constexpr PinMask None = PinMask(0);
//     static constexpr PinMask _0 =   PinMask::from_index(0);
//     static constexpr PinMask _1 =   PinMask::from_index(1);
//     static constexpr PinMask _2 =   PinMask::from_index(2);
//     static constexpr PinMask _3 =   PinMask::from_index(3);
//     static constexpr PinMask _4 =   PinMask::from_index(4);
//     static constexpr PinMask _5 =   PinMask::from_index(5);
//     static constexpr PinMask _6 =   PinMask::from_index(6);
//     static constexpr PinMask _7 =   PinMask::from_index(7);
//     static constexpr PinMask _8 =   PinMask::from_index(8);
//     static constexpr PinMask _9 =   PinMask::from_index(9);
//     static constexpr PinMask _10 =  PinMask::from_index(10);
//     static constexpr PinMask _11 =  PinMask::from_index(11);
//     static constexpr PinMask _12 =  PinMask::from_index(12);
//     static constexpr PinMask _13 =  PinMask::from_index(13);
//     static constexpr PinMask _14 =  PinMask::from_index(14);
//     static constexpr PinMask _15 =  PinMask::from_index(15);
// };

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