#pragma once

#include <cstdint>
#include "core/platform.hpp"
#include "core/utils/nth.hpp"
#include "primitive/gpio/pin_source.hpp"

namespace ymd::hal{


struct [[nodiscard]] PinMask{
public:
    [[nodiscard]] constexpr PinMask(const PinSource nth):
        bits_(std::bit_cast<uint16_t>(nth)){;}

    [[nodiscard]] static constexpr PinMask zero(){
        return PinMask(0);
    }
    [[nodiscard]] static constexpr PinMask from_u16(const uint16_t bits){
        return PinMask(bits);
    }

    [[nodiscard]] static constexpr PinMask from_nth(const Nth nth){
        return PinMask::from_u16(uint16_t(1 << nth.count()));
    }
    [[nodiscard]] constexpr uint16_t to_u16() const {return bits_;}

    [[nodiscard]] constexpr bool test(Nth nth) const {
        return bits_ & (1 << nth.count());
    }

    [[nodiscard]] constexpr PinMask modify(Nth nth, const BoolLevel level) const {
        if(level == HIGH) return PinMask(bits_ | (1 << nth.count()));
        else return PinMask(bits_ & (~(1 << nth.count())));
    }

    [[nodiscard]] constexpr PinMask set_bit(Nth nth) const {
        return PinMask(bits_ | (1 << nth.count()));
    }

    [[nodiscard]] constexpr PinMask clr_bit(Nth nth) const {
        return PinMask(bits_ & (~(1 << nth.count())));
    }

    [[nodiscard]] constexpr PinMask operator | (const PinMask other) const {
        return PinMask(bits_ | other.bits_);
    }

    [[nodiscard]] constexpr PinMask operator & (const PinMask other) const {
        return PinMask(bits_ & other.bits_);
    }

    [[nodiscard]] constexpr PinMask operator ~() const {
        return PinMask(~bits_);
    }

    [[nodiscard]] constexpr bool operator == (const PinMask & other) const{
        return  bits_ == other.bits_;
    }

    [[nodiscard]] explicit constexpr operator bool() const {
        return bits_;
    }

    [[nodiscard]] constexpr bool any() const {
        return bits_;
    }

    struct [[nodiscard]] Iterator final{
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

    [[nodiscard]] constexpr Iterator iter() const {
        return Iterator{bits_};
    }
private:
    uint16_t bits_;

    [[nodiscard]] explicit constexpr PinMask(const uint16_t bits):
        bits_(bits){;}

};


enum class [[nodiscard]] PinName:uint8_t{
    #define PINNAME_CREATE_TEMPLATE(x, n)\
    P##x##0 = n | 0b00000, P##x##1, P##x##2, P##x##3, P##x##4, P##x##5, P##x##6, P##x##7,\
    P##x##8, P##x##9, P##x##10, P##x##11, P##x##12, P##x##13, P##x##14, P##x##15,\
    P##x##16, P##x##17, P##x##18, P##x##19, P##x##20, P##x##21, P##x##22, P##x##23,\
    P##x##24, P##x##25, P##x##26, P##x##27, P##x##28, P##x##29, P##x##30, P##x##31\

    #ifdef GPIOA_PRESENT
    PINNAME_CREATE_TEMPLATE(A, 0)
    #endif

    #ifdef GPIOB_PRESENT
    ,
    PINNAME_CREATE_TEMPLATE(B, 1)
    #endif

    #ifdef GPIOC_PRESENT
    ,
    PINNAME_CREATE_TEMPLATE(C, 2)
    #endif

    #ifdef GPIOD_PRESENT
    ,
    PINNAME_CREATE_TEMPLATE(D, 3)
    #endif

    #ifdef GPIOE_PRESENT
    ,
    PINNAME_CREATE_TEMPLATE(E, 4)
    #endif

    #ifdef GPIOF_PRESENT
    ,
    PINNAME_CREATE_TEMPLATE(F, 5)
    #endif

    #ifdef GPIOG_PRESENT
    ,
    PINNAME_CREATE_TEMPLATE(G, 6)
    #endif
};

class [[nodiscard]] GpioMode{
public:
    enum class [[nodiscard]] Kind:uint8_t{
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

    [[nodiscard]] constexpr bool operator ==(const Kind kind) const 
        {return kind_ == kind;}
    [[nodiscard]] constexpr bool operator ==(const GpioMode other) const
        {return kind_ == other.kind_;}
    [[nodiscard]] constexpr bool is_input() const {
        return (kind_ == InAnalog) 
        || (kind_ == InFloating) 
        || (kind_ == InPullUP) 
        || (kind_ == InPullDN);
    }

    [[nodiscard]] constexpr bool is_output() const {
        return (kind_ == OutPP)
        || (kind_ == OutOD)
        || (kind_ == OutAfPP)
        || (kind_ == OutAfOD);
    }

    [[nodiscard]] constexpr bool is_outpp() const {
        return (kind_ == OutPP)
        || (kind_ == OutAfPP);
    }

    [[nodiscard]] constexpr bool is_outod() const {
        return (kind_ == OutOD)
        || (kind_ == OutAfOD);
    }

    [[nodiscard]] constexpr uint8_t to_u8() const {
        return std::bit_cast<uint8_t>(kind_);
    }
private:
    Kind kind_;
};

}