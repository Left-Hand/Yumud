#pragma once

#include "port.hpp"
#include "constants.hpp"

#include "div.hpp"

#ifndef M_PI
#define M_PI (3.1415926536)
#endif


namespace ymd::fxmath::details{

struct alignas(4) [[nodiscard]] Atan2Flag final{
    using Self = Atan2Flag;
    uint8_t y_is_neg;
    uint8_t x_is_neg;
    uint8_t swapped;

    __attribute__((always_inline,  optimize( "-Ofast" )))
    static constexpr Self zero(){
        return std::bit_cast<Self>(uint32_t(0));
    }

    __attribute__((always_inline,  optimize( "-Ofast" )))
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
    __attribute__((always_inline,  optimize( "-Ofast" )))
    [[nodiscard]] static constexpr uint32_t transfrom_pu_x_to_uq32_result(uint32_t uq32_input) {
        // uq32_input ∈ [0, 1)


        const auto & piq32_coeffs = fxmath::details::IQ32ATAN_COEFFS[(uq32_input >> 27)];
        // __builtin_prefetch(&piq32_coeffs[0], );
        /*
        * Calculate atan(x) using the following Taylor series:
        *
        *     atan(x) = ((c3*x + c2)*x + c1)*x + c0
        */

        /* c3*x */
        uint32_t uq32_result_pu = intrinsics::mul32hsu(piq32_coeffs[0], uq32_input);

        /* c3*x + c2 */
        uq32_result_pu = uq32_result_pu + piq32_coeffs[1];

        /* (c3*x + c2)*x */
        uq32_result_pu = intrinsics::mul32hsu(static_cast<int32_t>(uq32_result_pu), uq32_input);

        /* (c3*x + c2)*x + c1 */
        uq32_result_pu = uq32_result_pu + piq32_coeffs[2];

        /* ((c3*x + c2)*x + c1)*x */
        uq32_result_pu = intrinsics::mul32hsu(static_cast<int32_t>(uq32_result_pu), uq32_input);

        /* ((c3*x + c2)*x + c1)*x + c0 */
        uq32_result_pu = uq32_result_pu + piq32_coeffs[3];
        return uq32_result_pu;
    }
};



__attribute__((always_inline,  optimize( "-Ofast" )))
constexpr ymd::math::fixed<32, uint32_t> atan2pu32(
    uint32_t uiqn_y,
    uint32_t uiqn_x
){
    Atan2Flag flag = Atan2Flag::zero();
    uint32_t uq32_input;

    if (uiqn_y & (1U << 31)) {
        flag.y_is_neg = 1;
        uiqn_y = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uiqn_y));
    }

    if (uiqn_x & (1U << 31)) {
        flag.x_is_neg = 1;
        uiqn_x = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uiqn_x));
    }

    if (uiqn_x < uiqn_y) {
        flag.swapped = 1;
        uq32_input = std::bit_cast<uint32_t>(ymd::fxmath::details::div32u<31>(
            uiqn_x, uiqn_y)) << (1);
    } else if((uiqn_x > uiqn_y)) {
        uq32_input = std::bit_cast<uint32_t>(ymd::fxmath::details::div32u<31>(
            uiqn_y, uiqn_x)) << (1);
    } else{
        // 1/8 lap
        // 1/8 * 2^32
        return flag.apply_to_uq32(((1u << (32 - 3))));
    }
    const uint32_t uq32_result_pu = Atan2Intermediate::transfrom_pu_x_to_uq32_result(uq32_input);
    return flag.apply_to_uq32(uq32_result_pu);
}

template<size_t Q>
__attribute__((always_inline,  optimize( "-Ofast" )))
constexpr ymd::math::fixed<32, uint32_t> atanpu32(
    uint32_t uiqn_y
){
    Atan2Flag flag = Atan2Flag::zero();
    uint32_t uq32_input;

    if (uiqn_y & (1U << 31)) {
        flag.y_is_neg = 1;
        uiqn_y = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uiqn_y));
    }

    constexpr uint32_t ONE_BITS = (1u << Q);

    if (uiqn_y > ONE_BITS) {
        flag.swapped = 1;
        //TODO 替换为更轻量的求倒数算法
        uq32_input = std::bit_cast<uint32_t>(ymd::fxmath::details::div32u<31>(
            (1u << Q), uiqn_y
        )) << 1;
    } else if(uiqn_y < ONE_BITS) {
        uq32_input = uiqn_y << (32 - Q);
        // uq32_input = (1u << (30));
    } else{// uiqn_y == ONE_BITS
        return flag.apply_to_uq32(((1u << (29))));
    }

    const uint32_t uq32_result_pu = Atan2Intermediate::transfrom_pu_x_to_uq32_result(uq32_input);
    return flag.apply_to_uq32(uq32_result_pu);
}

}

namespace ymd::math{



template<size_t Q>
constexpr ymd::math::fixed<32, uint32_t> atan2pu(
    ymd::math::fixed<Q, int32_t> iqn_input_y, 
    ymd::math::fixed<Q, int32_t> iqn_input_x
){
    return fxmath::details::atan2pu32(iqn_input_y.to_bits(), iqn_input_x.to_bits());
}



template<size_t Q>
constexpr ymd::math::fixed<29, int32_t> atan2(
    ymd::math::fixed<Q, int32_t> iqn_input_y, 
    ymd::math::fixed<Q, int32_t> iqn_input_x
){
    return ymd::math::uq32_to_rad(atan2pu<Q>(iqn_input_y, iqn_input_x));
}

template<size_t Q>
constexpr ymd::math::fixed<32, uint32_t> atanpu(
    ymd::math::fixed<Q, int32_t> iqn_input_y
){
    return fxmath::details::atanpu32<Q>(iqn_input_y.to_bits());
}


template<size_t Q>
constexpr ymd::math::fixed<29, int32_t> atan(
    ymd::math::fixed<Q, int32_t> iqn_input_y
){
    return ymd::math::uq32_to_rad(atanpu<Q>(iqn_input_y));
}

}