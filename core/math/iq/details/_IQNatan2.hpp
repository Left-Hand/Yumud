#pragma once

#include "support.hpp"
#include "_IQNtables.hpp"

#include "_IQNdiv.hpp"

#ifndef M_PI
#define M_PI (3.1415926536)
#endif

#define TYPE_PU         (0)
#define TYPE_RAD        (1)


namespace ymd::iqmath::details{

struct [[nodiscard]] Atan2Flag{
    /*
    * Extract the sign from the inputs and set the following flag bits:
    *
    *      flag = xxxxxTQS
    *      x = unused
    *      T = transform was inversed
    *      Q = 2nd or 3rd quadrant (-x)
    *      S = sign bit needs to be set (-y)
    */

    uint8_t y_is_neg:1;
    uint8_t x_is_neg:1;
    uint8_t inversed:1;

    static constexpr Atan2Flag zero(){
        return Atan2Flag{0, 0, 0};
    }


    template<size_t Q, int type>
    [[nodiscard]] constexpr int32_t transform_uiq32pu(uint32_t uiq32ResultPU) const {
        auto & self = *this;
        int32_t iqNResult;
        int32_t iq29Result;

        /* Check if we inversed the transformation. */
        if (self.inversed) {
            /* atan(y/x) = pi/2 - uiq32ResultPU */
            uiq32ResultPU = (uint32_t)(0x40000000 - uiq32ResultPU);
        }

        /* Check if the result needs to be mirrored to the 2nd/3rd quadrants. */
        if (self.x_is_neg) {
            /* atan(y/x) = pi - uiq32ResultPU */
            uiq32ResultPU = (uint32_t)(0x80000000 - uiq32ResultPU);
        }

        /* Round and convert result to correct format (radians/PU and iqN type). */
        if constexpr(type ==  TYPE_PU) {
            uiq32ResultPU += (uint32_t)1 << (31 - Q);
            iqNResult = uiq32ResultPU >> (32 - Q);
        } else {
            /*
            * Multiply the per-unit result by 2*pi:
            *
            *     iq31mpy(iq32, iq28) = iq29
            */
            iq29Result = __mpyf_l(uiq32ResultPU, iqmath::details::_iq28_twoPi);

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
        if (self.y_is_neg) {
            return -iqNResult;
        } else {
            return iqNResult;
        }
    };
};

struct [[nodiscard]] Atan2Intermediate{
    using Self = Atan2Intermediate;


    // * Calculate atan2 using a 3rd order Taylor series. The coefficients are stored
    // * in a lookup table with 17 ranges to give an accuracy of XX bits.
    // *
    // * The input to the Taylor series is the ratio of the two inputs and must be
    // * in the range of 0 <= input <= 1. If the y argument is larger than the x
    // * argument we must apply the following transformation:
    // *
    // *     atan(y/x) = pi/2 - atan(x/y)
    // */
    static constexpr uint32_t transform_input_to_result(uint32_t uiq31Input) {
        const auto * piq32Coeffs = &iqmath::details::_IQ32atan_coeffs[(uiq31Input >> 24) & 0x00fc];
        /*
        * Calculate atan(x) using the following Taylor series:
        *
        *     atan(x) = ((c3*x + c2)*x + c1)*x + c0
        */

        /* c3*x */
        uint32_t uiq32ResultPU = __mpyf_l(uiq31Input, *piq32Coeffs++);

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
        return uiq32ResultPU;
    }
};


template<const size_t Q, const uint8_t type>
constexpr int32_t __IQNatan2_impl(uint32_t uiqNInputY, uint32_t uiqNInputX){
    Atan2Flag flag = Atan2Flag::zero();
    uint32_t uiq31Input;

    if (uiqNInputY & (1U << 31)) {
        flag.y_is_neg = 1;
        uiqNInputY = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uiqNInputY));
    }
    if (uiqNInputX & (1U << 31)) {
        flag.x_is_neg = 1;
        uiqNInputX = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uiqNInputX));
    }

    /*
    * Calcualte the ratio of the inputs in iq31. When using the iq31 div
    * fucntions with inputs of matching type the result will be iq31:
    *
    *     iq31 = _IQ31div(iqN, iqN);
    */
    if (uiqNInputX < uiqNInputY) {
        flag.inversed = 1;
        uiq31Input = std::bit_cast<uint32_t>(iqmath::details::__IQNdiv_impl<31, true>(
            uiqNInputX, uiqNInputY));
    } else if((uiqNInputX > uiqNInputY)) {
        uiq31Input = std::bit_cast<uint32_t>(iqmath::details::__IQNdiv_impl<31, true>(
            uiqNInputY, uiqNInputX));
    } else{
        // 1/8 lap
        // 1/8 * 2^32
        return flag.template transform_uiq32pu<Q, type>(((1u << (32 - 3))));
    }
    
    const uint32_t uiq32ResultPU = Atan2Intermediate::transform_input_to_result(uiq31Input);
    return flag.template transform_uiq32pu<Q, type>(uiq32ResultPU);
}

template<const size_t Q, const uint8_t type>
constexpr int32_t __IQNatan_impl(uint32_t uiqNInputX){
    return __IQNatan2_impl<Q, type>(uiqNInputX, (1u << Q));
}


template<const size_t Q>
constexpr math::fixed_t<Q, int32_t> _IQNatan2(
    math::fixed_t<Q, int32_t> iqNInputY, 
    math::fixed_t<Q, int32_t> iqNInputX
){
    return math::fixed_t<Q, int32_t>::from_bits(__IQNatan2_impl<Q, TYPE_RAD>(
        std::bit_cast<uint32_t>(iqNInputY.to_bits()), 
        std::bit_cast<uint32_t>(iqNInputX.to_bits()))
    );
}

template<const size_t Q>
constexpr math::fixed_t<Q, int32_t> _IQNatan2PU(
    math::fixed_t<Q, int32_t> iqNInputY, 
    math::fixed_t<Q, int32_t> iqNInputX
){
    return math::fixed_t<Q, int32_t>::from_bits(__IQNatan2_impl<Q, TYPE_PU>(
        std::bit_cast<uint32_t>(iqNInputY.to_bits()), 
        std::bit_cast<uint32_t>(iqNInputX.to_bits()))
    );
}

template<const size_t Q>
constexpr math::fixed_t<Q, int32_t> _IQNatan2(math::fixed_t<Q, int32_t> iqNInputX){
    return math::fixed_t<Q, int32_t>::from_bits(__IQNatan_impl<Q, TYPE_RAD>(
        std::bit_cast<uint32_t>(iqNInputX.to_bits()))
    );
}

template<const size_t Q>
constexpr math::fixed_t<Q, int32_t> _IQNatan2PU(math::fixed_t<Q, int32_t> iqNInputX){
    return math::fixed_t<Q, int32_t>::from_bits(__IQNatanPU_impl<Q, TYPE_PU>(
        std::bit_cast<uint32_t>(iqNInputX.to_bits()))
    );
}

}

#undef TYPE_PU
#undef TYPE_RAD