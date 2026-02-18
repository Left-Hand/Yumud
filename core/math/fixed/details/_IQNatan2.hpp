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

struct alignas(4) [[nodiscard]] Atan2Flag final{
    using Self = Atan2Flag;
    uint8_t y_is_neg;
    uint8_t x_is_neg;
    uint8_t swapped;

    static constexpr Self zero(){
        return std::bit_cast<Self>(uint32_t(0));
    }


    [[nodiscard]] constexpr ymd::math::fixed<32, uint32_t> apply_to_uq32(uint32_t uq32_result_pu) const {
        auto & self = *this;

        /* Check if we swapped the transformation. */
        if (self.swapped) {
            /* atan(y/x) = pi/2 - uq32_result_pu */
            uq32_result_pu = (uint32_t)(0x40000000u - uq32_result_pu);
        }

        /* Check if the result needs to be mirrored to the 2nd/3rd quadrants. */
        if (self.x_is_neg) {
            /* atan(y/x) = pi - uq32_result_pu */
            uq32_result_pu = (uint32_t)(0x80000000u - uq32_result_pu);
        }


        /* Set the sign bit and result to correct quadrant. */
        if (self.y_is_neg) {
            uq32_result_pu = ~uq32_result_pu;
        }

        return ymd::math::fixed<32, uint32_t>::from_bits(uq32_result_pu);
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
    static constexpr uint32_t transfrom_pu_x_to_uq32_result(uint32_t uq32_input) {
        // return 0;
        const int32_t * piq32Coeffs = &ymd::iqmath::details::IQ32ATAN_COEFFS[(uq32_input >> 25) & 0x00fc];
        /*
        * Calculate atan(x) using the following Taylor series:
        *
        *     atan(x) = ((c3*x + c2)*x + c1)*x + c0
        */

        /* c3*x */
        uint32_t uq32_result_pu = fast_mul(uq32_input, piq32Coeffs[0]);

        /* c3*x + c2 */
        uq32_result_pu = uq32_result_pu + piq32Coeffs[1];

        /* (c3*x + c2)*x */
        uq32_result_pu = fast_mul(uq32_input, uq32_result_pu);

        /* (c3*x + c2)*x + c1 */
        uq32_result_pu = uq32_result_pu + piq32Coeffs[2];

        /* ((c3*x + c2)*x + c1)*x */
        uq32_result_pu = fast_mul(uq32_input, uq32_result_pu);

        /* ((c3*x + c2)*x + c1)*x + c0 */
        uq32_result_pu = uq32_result_pu + piq32Coeffs[3];
        return uq32_result_pu;
    }

private:
    [[nodiscard]] __attribute__((__always_inline__)) 
    static constexpr int32_t fast_mul(uint32_t arg1, int32_t arg2){
        return uint32_t((uint64_t(arg1) * uint64_t(arg2)) >> 32);
    }
};



template<size_t Q>
constexpr ymd::math::fixed<32, uint32_t> _atan2pu_impl(
    uint32_t uqn_y,
    uint32_t uqn_x
){
    Atan2Flag flag = Atan2Flag::zero();
    uint32_t uq32_input;

    if (uqn_y & (1U << 31)) {
        flag.y_is_neg = 1;
        uqn_y = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uqn_y));
    }

    if (uqn_x & (1U << 31)) {
        flag.x_is_neg = 1;
        uqn_x = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uqn_x));
    }

    if (uqn_x < uqn_y) {
        flag.swapped = 1;
        uq32_input = std::bit_cast<uint32_t>(ymd::iqmath::details::__IQNdiv_impl<31, false>(
            uqn_x, uqn_y)) << (1);
    } else if((uqn_x > uqn_y)) {
        uq32_input = std::bit_cast<uint32_t>(ymd::iqmath::details::__IQNdiv_impl<31, false>(
            uqn_y, uqn_x)) << (1);
    } else{
        // 1/8 lap
        // 1/8 * 2^32
        return flag.apply_to_uq32(((1u << (29))));
    }
    const uint32_t uq32_result_pu = Atan2Intermediate::transfrom_pu_x_to_uq32_result(uq32_input);
    return flag.apply_to_uq32(uq32_result_pu);
}

template<size_t Q>
constexpr ymd::math::fixed<32, uint32_t> _atanpu_impl(
    uint32_t uqn_y
){
    Atan2Flag flag = Atan2Flag::zero();
    uint32_t uq32_input;

    if (uqn_y & (1U << 31)) {
        flag.y_is_neg = 1;
        uqn_y = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uqn_y));
    }

    constexpr uint32_t ONE_BITS = (1u << Q);

    if (uqn_y > ONE_BITS) {
        flag.swapped = 1;
        //TODO 替换为更轻量的求倒数算法
        uq32_input = std::bit_cast<uint32_t>(ymd::iqmath::details::__IQNdiv_impl<31, false>(
            (1u << Q), uqn_y
        )) << 1;
    } else if(uqn_y < ONE_BITS) {
        uq32_input = uqn_y << (32 - Q);
        // uq32_input = (1u << (30));
    } else{// uqn_y == ONE_BITS
        return flag.apply_to_uq32(((1u << (29))));
    }

    const uint32_t uq32_result_pu = Atan2Intermediate::transfrom_pu_x_to_uq32_result(uq32_input);
    return flag.apply_to_uq32(uq32_result_pu);
}

}

#undef TYPE_PU
#undef TYPE_RAD