#pragma once

#include "constants.hpp"
#include "port.hpp"

namespace ymd::fxmath::details{



template<size_t Q, bool IS_SIGNED>
__attribute__((optimize( "-Ofast" )))
constexpr int32_t iqn_div_impl(int32_t iqNInput1, int32_t iqNInput2)
{
    #if 0
    size_t ui8Index = 0;
    bool is_neg = 0;
    uint32_t uiq30Guess;
    uint32_t uiqNInput1;
    uint32_t uiq31Input2;
    uint32_t uiqNResult;
    uint64_t uiiqNInput1;

    if constexpr(IS_SIGNED == true) {
        /* save sign of denominator */
        if (iqNInput2 <= 0) {
            /* check for divide by zero */
            if (iqNInput2 == 0) {
                return INT32_MAX;
            } else {
                is_neg = 1;
                iqNInput2 = -iqNInput2;
            }
        }

        /* save sign of numerator */
        if (iqNInput1 < 0) {
            is_neg = !is_neg;
            iqNInput1 = -iqNInput1;
        }
    } else {
        /* Check for divide by zero */
        if (iqNInput2 == 0) {
            return INT32_MAX;
        }
    }

    /* Save input1 and input2 to unsigned IQN and IIQN (64-bit). */
    uiiqNInput1 = (uint64_t)iqNInput1;
    uiq31Input2 = (uint32_t)iqNInput2;

    /* Scale inputs so that 0.5 <= uiq31Input2 < 1.0. */
    while (uiq31Input2 < 0x40000000) {
        uiq31Input2 <<= 1;
        uiiqNInput1 <<= 1;
    }

    /*
     * Shift input1 back from iq31 to iqN but scale by 2 since we multiply
     * by result in iq30 format.
     */
    if constexpr(Q < 31) {
        uiiqNInput1 >>= (31 - Q - 1);
    } else {
        uiiqNInput1 <<= 1;
    }

    /* Check for saturation. */
    if (uiiqNInput1 >> 32) {
        if (is_neg) {
            return INT32_MIN;
        } else {
            return INT32_MAX;
        }
    } else {
        uiqNInput1 = (uint32_t)uiiqNInput1;
    }

    #else
    bool is_neg = 0;
    uint32_t uiqNResult;

    if constexpr(IS_SIGNED == true) {
        /* save sign of denominator */
        if (iqNInput2 == 0) [[unlikely]]{
            return INT32_MAX;
        }else if(iqNInput2 < 0){
            if(iqNInput2 == INT32_MIN) [[unlikely]] {
                iqNInput2 = INT32_MAX;
                is_neg = 1;
            }else{
                iqNInput2 = -iqNInput2;
                is_neg = 1;
            }
        }

        /* save sign of numerator */
        if (iqNInput1 < 0) {
            is_neg = !is_neg;

            if(iqNInput1 == INT32_MIN) [[unlikely]] {
                iqNInput1 = INT32_MAX;
            }else{
                iqNInput1 = -iqNInput1;
            }
        }

    } else {
        /* Check for divide by zero */
        if (iqNInput2 == 0) [[unlikely]] {
            return INT32_MAX;
        }
    }


    /* Scale inputs so that 0.5 <= uiq32Input2 < 1.0. */
    // Handle zero case to avoid undefined behavior in __builtin_clz
    // Find the number of leading zeros to determine the shift amount
    #if 0
    //1.046us per call @ch32v303 144mhz(fpu present)
    #if 0
    const size_t shift_amount = [&] -> size_t __no_inline{
        return size_t(CLZ(iqNInput2));
    }();
    #else
    const size_t shift_amount = size_t(CLZ(iqNInput2));
    #endif
    #else
    //0.79us per call @ch32v303 144mhz(fpu present)
    const size_t shift_amount = __builtin_clz(iqNInput2);
    #endif

    if(shift_amount >= 32) __builtin_unreachable();
    
    uint32_t uiq32Input2 = iqNInput2 << shift_amount;
    uint64_t uiiqNInput1 = uint64_t(iqNInput1);
    if constexpr(Q < 31) {
        const int32_t shifts = (31 - Q - shift_amount);
        if(shifts >= 0) {
            uiiqNInput1 >>= shifts;
        } else {
            uiiqNInput1 <<= -shifts;
        }
    } else {
        uiiqNInput1 <<= (Q - 31) + shift_amount;
    }

    size_t ui8Index = 0;
    uint32_t uiq30Guess;
    uint32_t uiqNInput1 = (uint32_t)uiiqNInput1;
    uint32_t uiq31Input2 = uiq32Input2 >> 1;
    // uint32_t uiqNResult;
    // uint64_t uiiqNInput1;
    #endif

    /* use left most 7 bits as ui8Index into lookup table (range: 32-64) */
    ui8Index = uiq31Input2 >> 24;
    ui8Index -= 64;
    uiq30Guess = (uint32_t)IQ6DIV_LOOPUP[ui8Index] << 24;



    /* 1st iteration */
    uint32_t ui30Temp = __mpyf_ul(uiq30Guess, uiq31Input2);
    ui30Temp = -((uint32_t)ui30Temp - 0x80000000);
    uiq30Guess = __mpyf_ul_reuse_arg1(uiq30Guess, ui30Temp << 1);

    /* 2nd iteration */
    ui30Temp = __mpyf_ul(uiq30Guess, uiq31Input2);
    ui30Temp = -((uint32_t)ui30Temp - 0x80000000);
    uiq30Guess = __mpyf_ul_reuse_arg1(uiq30Guess, ui30Temp << 1);

    /* 3rd iteration */
    ui30Temp = __mpyf_ul(uiq30Guess, uiq31Input2);
    ui30Temp = -((uint32_t)ui30Temp - 0x80000000);
    uiq30Guess = __mpyf_ul_reuse_arg1(uiq30Guess, ui30Temp << 1);

    /* Multiply 1/uiq31Input2 and uiqNInput1. */
    uiqNResult = __mpyf_ul(uiq30Guess, uiqNInput1);


    /* Saturate, add the sign and return. */
    if constexpr(IS_SIGNED == true) {
        if (uiqNResult > INT32_MAX) {
            if (is_neg) {
                return INT32_MIN;
            } else {
                return INT32_MAX;
            }
        } else {
            if (is_neg) {
                return -(int32_t)uiqNResult - 1;
            } else {
                return (int32_t)uiqNResult + 1;
            }
        }
    } else {
        return uiqNResult + 1;
    }
}

template<size_t Q>
__attribute__((optimize( "-Ofast" )))
constexpr int32_t div32i(int32_t iqNInput1, int32_t iqNInput2){
    return iqn_div_impl<Q, true>(iqNInput1, iqNInput2);
}

template<size_t Q>
__attribute__((optimize( "-Ofast" )))
constexpr uint32_t div32u(uint32_t iqNInput1, uint32_t iqNInput2){
    return std::bit_cast<uint32_t>(iqn_div_impl<Q, false>(
        std::bit_cast<int32_t>(iqNInput1), 
        std::bit_cast<int32_t>(iqNInput2)
    ));
}

}
