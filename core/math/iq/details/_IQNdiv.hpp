#pragma once

#include "support.hpp"
#include "_IQNtables.hpp"


namespace ymd::iqmath::details{


template<int8_t Q, bool _is_signed>
constexpr int32_t __IQNdiv_impl(int32_t iqNInput1, int32_t iqNInput2)
{
    size_t ui8Index, ui8Sign = 0;
    uint32_t ui32Temp;
    uint32_t uiq30Guess;
    uint32_t uiqNInput1;
    uint32_t uiqNInput2;
    uint32_t uiqNResult;
    uint64_t uiiqNInput1;

    if constexpr(_is_signed == true) {
        /* save sign of denominator */
        if (iqNInput2 <= 0) {
            /* check for divide by zero */
            if (iqNInput2 == 0) {
                return INT32_MAX;
            } else {
                ui8Sign = 1;
                iqNInput2 = -iqNInput2;
            }
        }

        /* save sign of numerator */
        if (iqNInput1 < 0) {
            ui8Sign ^= 1;
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
    uiqNInput2 = (uint32_t)iqNInput2;

    /* Scale inputs so that 0.5 <= uiqNInput2 < 1.0. */
    while (uiqNInput2 < 0x40000000) {
        uiqNInput2 <<= 1;
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
        if (ui8Sign) {
            return INT32_MIN;
        } else {
            return INT32_MAX;
        }
    } else {
        uiqNInput1 = (uint32_t)uiiqNInput1;
    }

    /* use left most 7 bits as ui8Index into lookup table (range: 32-64) */
    ui8Index = uiqNInput2 >> 24;
    ui8Index -= 64;
    uiq30Guess = (uint32_t)_IQ6div_lookup[ui8Index] << 24;



    /* 1st iteration */
    ui32Temp = __mpyf_ul(uiq30Guess, uiqNInput2);
    ui32Temp = -((uint32_t)ui32Temp - 0x80000000);
    ui32Temp = ui32Temp << 1;
    uiq30Guess = __mpyf_ul_reuse_arg1(uiq30Guess, ui32Temp);

    /* 2nd iteration */
    ui32Temp = __mpyf_ul(uiq30Guess, uiqNInput2);
    ui32Temp = -((uint32_t)ui32Temp - 0x80000000);
    ui32Temp = ui32Temp << 1;
    uiq30Guess = __mpyf_ul_reuse_arg1(uiq30Guess, ui32Temp);

    /* 3rd iteration */
    ui32Temp = __mpyf_ul(uiq30Guess, uiqNInput2);
    ui32Temp = -((uint32_t)ui32Temp - 0x80000000);
    ui32Temp = ui32Temp << 1;
    uiq30Guess = __mpyf_ul_reuse_arg1(uiq30Guess, ui32Temp);

    /* Multiply 1/uiqNInput2 and uiqNInput1. */
    uiqNResult = __mpyf_ul(uiq30Guess, uiqNInput1);


    /* Saturate, add the sign and return. */
    if constexpr(_is_signed == true) {
        if (uiqNResult > INT32_MAX) {
            if (ui8Sign) {
                return INT32_MIN;
            } else {
                return INT32_MAX;
            }
        } else {
            if (ui8Sign) {
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
