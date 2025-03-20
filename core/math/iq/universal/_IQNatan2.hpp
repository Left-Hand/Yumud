#pragma once

#include "support.hpp"
#include "_IQNtables.hpp"

#include "_IQNdiv.hpp"

#ifndef PI
#define PI (3.1415926536)
#endif

#define TYPE_PU         (0)
#define TYPE_RAD        (1)


namespace __iqdetails{
/**
 * @brief Compute the 4-quadrant arctangent of the IQN input
 *        and return the result.
 *
 * @param iqNInputY       IQN type input y.
 * @param iqNInputX       IQN type input x.
 * @param type            Specifies radians or per-unit operation.
 * @param Q         IQ format.
 *
 * @return                IQN type result of 4-quadrant arctangent.
 */
/*
 * Calculate atan2 using a 3rd order Taylor series. The coefficients are stored
 * in a lookup table with 17 ranges to give an accuracy of XX bits.
 *
 * The input to the Taylor series is the ratio of the two inputs and must be
 * in the range of 0 <= input <= 1. If the y argument is larger than the x
 * argument we must apply the following transformation:
 *
 *     atan(y/x) = pi/2 - atan(x/y)
 */
template<const size_t Q, const uint8_t type>
constexpr int32_t __IQNatan2_impl(int32_t iqNInputY, int32_t iqNInputX)
{
    uint8_t ui8Status = 0;
    uint8_t ui8Index;

    uint32_t uiqNInputX;
    uint32_t uiqNInputY;
    uint32_t uiq32ResultPU;
    int32_t iqNResult;
    int32_t iq29Result;
    const int32_t *piq32Coeffs;
    uint32_t uiq31Input;

    /*
     * Extract the sign from the inputs and set the following status bits:
     *
     *      ui8Status = xxxxxTQS
     *      x = unused
     *      T = transform was applied
     *      Q = 2nd or 3rd quadrant (-x)
     *      S = sign bit needs to be set (-y)
     */
    if (iqNInputY < 0) {
        ui8Status |= 1;
        iqNInputY = -iqNInputY;
    }
    if (iqNInputX < 0) {
        ui8Status |= 2;
        iqNInputX = -iqNInputX;
    }

    /* Save inputs to unsigned iqN formats. */
    uiqNInputX = (uint32_t)iqNInputX;
    uiqNInputY = (uint32_t)iqNInputY;

    /*
     * Calcualte the ratio of the inputs in iq31. When using the iq31 div
     * fucntions with inputs of matching type the result will be iq31:
     *
     *     iq31 = _IQ31div(iqN, iqN);
     */
    if (uiqNInputX < uiqNInputY) {
        ui8Status |= 4;
        uiq31Input = std::bit_cast<uint32_t>(_UIQdiv<31>(
            _iq<31>::from_i32(uiqNInputX), _iq<31>::from_i32(uiqNInputY)));
    } else {
        uiq31Input = std::bit_cast<uint32_t>(_UIQdiv<31>(
            _iq<31>::from_i32(uiqNInputY), _iq<31>::from_i32(uiqNInputX)));
    }

    /* Calculate the index using the left 8 most bits of the input. */
    ui8Index = (uint16_t)(uiq31Input >> 24);
    ui8Index = ui8Index & 0x00fc;

    /* Set the coefficient pointer. */
    piq32Coeffs = &_IQ32atan_coeffs[ui8Index];


    /*
     * Calculate atan(x) using the following Taylor series:
     *
     *     atan(x) = ((c3*x + c2)*x + c1)*x + c0
     */

    /* c3*x */
    uiq32ResultPU = __mpyf_l(uiq31Input, *piq32Coeffs++);

    /* c3*x + c2 */
    uiq32ResultPU = uiq32ResultPU + *piq32Coeffs++;

    /* (c3*x + c2)*x */
    uiq32ResultPU = __mpyf_l(uiq31Input, uiq32ResultPU);

    /* (c3*x + c2)*x + c1 */
    uiq32ResultPU = uiq32ResultPU + *piq32Coeffs++;

    /* ((c3*x + c2)*x + c1)*x */
    uiq32ResultPU = __mpyf_l(uiq31Input, uiq32ResultPU);

    /* ((c3*x + c2)*x + c1)*x + c0 */
    uiq32ResultPU = uiq32ResultPU + *piq32Coeffs++;

    /* Check if we applied the transformation. */
    if (ui8Status & 4) {
        /* atan(y/x) = pi/2 - uiq32ResultPU */
        uiq32ResultPU = (uint32_t)(0x40000000 - uiq32ResultPU);
    }

    /* Check if the result needs to be mirrored to the 2nd/3rd quadrants. */
    if (ui8Status & 2) {
        /* atan(y/x) = pi - uiq32ResultPU */
        uiq32ResultPU = (uint32_t)(0x80000000 - uiq32ResultPU);
    }

    /* Round and convert result to correct format (radians/PU and iqN type). */
    if constexpr(type == TYPE_PU) {
        uiq32ResultPU += (uint32_t)1 << (31 - Q);
        iqNResult = uiq32ResultPU >> (32 - Q);
    } else {
        /*
         * Multiply the per-unit result by 2*pi:
         *
         *     iq31mpy(iq32, iq28) = iq29
         */
        iq29Result = __mpyf_l(uiq32ResultPU, _iq28_twoPi);

        /* Only round IQ formats < 29 */
        if constexpr(Q < 29) {
            iq29Result += (uint32_t)1 << (28 - Q);
            iqNResult = iq29Result >> (29 - Q);
        }else if constexpr (Q == 29){
            iqNResult = iq29Result;
        }else{
            iqNResult = iq29Result << (Q - 29);
        }
    }


    /* Set the sign bit and result to correct quadrant. */
    if (ui8Status & 1) {
        return -iqNResult;
    } else {
        return iqNResult;
    }
}


template<const size_t Q>
constexpr _iq<Q> _IQNatan2(_iq<Q> iqNInputY, _iq<Q> iqNInputX){
    return _iq<Q>::from_i32(__IQNatan2_impl<Q, TYPE_RAD>(
        std::bit_cast<int32_t>(iqNInputY), 
        std::bit_cast<int32_t>(iqNInputX))
    );
}

template<const size_t Q>
constexpr _iq<Q> _IQNatan2PU(_iq<Q> iqNInputY, _iq<Q> iqNInputX){
    return _iq<Q>::from_i32(__IQNatan2_impl<Q, TYPE_PU>(
        std::bit_cast<int32_t>(iqNInputY), 
        std::bit_cast<int32_t>(iqNInputX))
    );
}

}

#undef TYPE_PU
#undef TYPE_RAD