#pragma once

#include "rts_support.h"

/* Common value defines. */
#define _q15_ln2          0x58b9
#define _q13_pi           0x6488
#define _q14_pi           0xc910
#define _q14_halfPi       0x6488
#define _q14_quarterPi    0x3244
#define _q15_halfPi       0xc910
#define _q15_quarterPi    0x6488
#define _q15_invRoot2     0x5a82
#define _q15_tanSmall     0x0021
#define _q15_pointOne     0x0ccd
#define _q15_oneTenth     0x0ccd
#define _iq28_twoPi       0x6487ed51
#define _iq29_pi          0x6487ed51
#define _iq29_halfPi      0x3243f6a8
#define _iq30_pi          0xc90fdaa2
#define _iq30_halfPi      0x6487ed51
#define _iq30_quarterPi   0x3243f6a8
#define _iq31_halfPi      0xc90fdaa2
#define _iq31_quarterPi   0x6487ed51
#define _iq31_invRoot2    0x5a82799a
#define _iq31_tanSmall    0x0020c49b
#define _iq31_ln2         0x58b90bfc
#define _iq31_twoThird    0x55555555
#define _iq31_pointOne    0x0ccccccd
#define _iq31_oneTenth    0x0ccccccd
#define _iq31_one         0x7fffffff


template<size_t Q>
requires (Q < 32)
struct _iq{
private:
    int32_t value_ = 0;
    __fast_inline constexpr _iq<Q>(const int32_t value):
        value_(value){
    }
public:
    operator int32_t() = delete;

    static __fast_inline constexpr _iq<Q> from_i32(const int32_t value){
        return _iq<Q>(value);
    }

    __fast_inline constexpr int32_t to_i32() const {
        return value_;
    }

    __fast_inline constexpr uint32_t to_u32() const {
        return std::bit_cast<uint32_t>(value_);
    }

    __fast_inline constexpr bool signbit() const{
        return value_ & 0x80000000;
    }
    __fast_inline constexpr operator bool() const{return value_;}

    template<size_t P>
    __fast_inline constexpr explicit operator _iq<P>() const{
        if constexpr (P > Q){
            return _iq<P>::from_i32(int32_t(value_) << (P - Q));
        }else if constexpr (P < Q){
            return _iq<P>::from_i32(int32_t(value_) >> (Q - P));
        }else{
            return _iq<P>::from_i32(int32_t(value_));
        }
    }
};