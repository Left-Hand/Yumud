#pragma once

#include "support.hpp"
#include "_IQNtables.hpp"


namespace ymd::iqmath::details{


template<int8_t Q, bool IS_SIGNED>
constexpr int32_t __IQNdiv_impl(int32_t iqNInput1, int32_t iqNInput2)
{
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

    /* use left most 7 bits as ui8Index into lookup table (range: 32-64) */
    ui8Index = uiq31Input2 >> 24;
    ui8Index -= 64;
    uiq30Guess = (uint32_t)_IQ6div_lookup[ui8Index] << 24;



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
                return -(int32_t)uiqNResult;
            } else {
                return (int32_t)uiqNResult;
            }
        }
    } else {
        return uiqNResult;
    }
}

}
