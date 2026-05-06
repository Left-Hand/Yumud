#pragma once

#include "port.hpp"
#include "constants.hpp"

#include "sqrt.hpp"

namespace ymd::fxmath::details{

/*
 * Calculate asin using a 4th order Taylor series for inputs in the range of
 * zero to 0.5. The coefficients are stored in a lookup table with 17 ranges
 * to give an accuracy of 26 bits.
 *
 * For inputs greater than 0.5 we apply the following transformation:
 *
 *     asin(x) = PI/2 - 2*asin(sqrt((1 - x)/2))
 *
 * This transformation is derived from the following trig identities:
 *
 *     (1) asin(x) = PI/2 - acos(x)
 *     (2) sin(t/2)^2 = (1 - cos(t))/2
 *     (3) cos(t) = x
 *     (4) t = acos(x)
 *
 * Identity (2) can be simplified to give equation (5):
 *
 *     (5) t = 2*asin(sqrt((1 - cos(t))/2))
 *
 * Substituing identities (3) and (4) into equation (5) gives equation (6):
 *
 *     (6) acos(x) = 2*asin(sqrt((1 - x)/2))
 *
 * The final step is substituting equation (6) into identity (1):
 *
 *     asin(x) = PI/2 - 2*asin(sqrt((1 - x)/2))
 *
 * Acos is implemented using asin and identity (1).
 */

__attribute__((always_inline, const, optimize("-Ofast")))
constexpr math::fixed<29, int32_t> asin31(uint32_t uiq31_input){
    bool is_acos = false;

    /*
     * Apply the transformation from asin to acos if input is greater than 0.5.
     * The first step is to calculate the following:
     *
     *     (sqrt((1 - uiq31_input)/2))
     */

    #if 0
    uint32_t uiq32_input;
    {
        const uint32_t Temp = 0x80000000u - uiq31_input;
        if (Temp < 0x40000000u) {
            /* Calculate sqrt((1 - uiq31_input)/2) */
            #if 0
            // uiq32_input = fxmath::details::sqrt32u<31>(math::fixed<31, uint32_t>::from_bits(Temp >> 1)).to_bits() << 1;
            #else
            // uiq32_input = fxmath::details::sqrt32u<31>(math::fixed<31, uint32_t>::from_bits(Temp >> 1)).to_bits() << 1;
            // uiq32_input = fxmath::details::sqrt32u<31>(math::fixed<32, uint32_t>::from_bits(Temp)).to_bits();
            uiq32_input = fxmath::details::sqrt32u<32>(math::fixed<32, uint32_t>::from_bits(Temp << 1)).to_bits();
            // uiq32_input = fxmath::details::sqrt32u<31>(math::fixed<31, uint32_t>::from_bits(Temp >> 1)).to_bits() << 1;
            #endif

            /* Flag that the transformation was used. */
            is_acos = true;
        }else{
            uiq32_input = uiq31_input << 1;
            is_acos = false;
        }
    }
    #else
    /*
     * Apply the transformation from asin to acos if input is greater than 0.5.
     * The first step is to calculate the following:
     *
     *     (sqrt((1 - uiq31_input)/2))
     */
    {
        const uint32_t uiq31_input_temp = 0x80000000u - uiq31_input;
        if (uiq31_input_temp < 0x40000000u) {
            /* Halve the result. */
            uiq31_input = uiq31_input_temp >> 1;

            /* Calculate sqrt((1 - uiq31_input)/2) */
            uiq31_input = fxmath::details::sqrt32u<31>(math::fixed<31, int32_t>::from_bits(uiq31_input)).to_bits();

            /* Flag that the transformation was used. */
            is_acos = true;
        }else{
            is_acos = false;
        }
    }

    if(uiq31_input & 0x80000000) __builtin_unreachable();
    const uint32_t uiq32_input = (uiq31_input << 1); 
    #endif
    
    /* Calculate the index using the left 6 most bits of the input. */
    /* Set the coefficient pointer. */
    const int32_t * piq29_coeffs = IQ29ASIN_COEFFS[size_t(uiq32_input >> 27)];
    int32_t iq29_result;

    /*
     * Calculate asin(x) using the following Taylor series:
     *
     *     asin(x) = (((c4*x + c3)*x + c2)*x + c1)*x + c0
     */

    /* c4*x */
    iq29_result = intrinsics::mul32hsu((piq29_coeffs[0]), uiq32_input);

    /* c4*x + c3 */
    iq29_result += piq29_coeffs[1];

    /* (c4*x + c3)*x */
    iq29_result = intrinsics::mul32hsu(iq29_result, uiq32_input);

    /* (c4*x + c3)*x + c2 */
    iq29_result += piq29_coeffs[2];

    /* ((c4*x + c3)*x + c2)*x */
    iq29_result = intrinsics::mul32hsu(iq29_result, uiq32_input);

    /* ((c4*x + c3)*x + c2)*x + c1 */
    iq29_result += piq29_coeffs[3];

    /* (((c4*x + c3)*x + c2)*x + c1)*x */
    iq29_result = intrinsics::mul32hsu(iq29_result, uiq32_input);

    /* (((c4*x + c3)*x + c2)*x + c1)*x + c0 */
    iq29_result += piq29_coeffs[4];

    /* check if we switched to acos */
    if (is_acos) {
        /* asin(x) = pi/2 - 2*iq29_result */

        // acos(x) = -(2 * asin(sqrt(1-x)/2)-pi/2)
        // 2\arcsin\left(\sqrt{\frac{\left(1-x\right)}{2}}\right)-\frac{\pi}{2}

        constexpr int32_t _iq29_halfPi = 0x3243f6a8;
        iq29_result = iq29_result << 1;
        iq29_result -= _iq29_halfPi;      // this is equivalent to the above
        iq29_result = -iq29_result;       // but avoids using temporary registers
    }

    return math::fixed<29, int32_t>::from_bits(iq29_result);

}

template<size_t Q>
__attribute__((always_inline, const, optimize("-Ofast"))) constexpr 
math::fixed<29, int32_t> asin32i(const math::fixed<Q, int32_t> x){
    static_assert(Q <= 32);
    uint32_t input_bits = std::bit_cast<uint32_t>(x.to_bits());
    const bool is_negative  = input_bits & (1u << 31);
    if(is_negative) input_bits = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(input_bits));

    constexpr uint32_t MAX_BITS = (uint32_t(1) << Q);
    if constexpr(Q < 32)
        if(input_bits > MAX_BITS) [[unlikely]]{
            input_bits = MAX_BITS;
        }

    #if 1
    const uint32_t uiq31_input = [&] -> uint32_t{
        if constexpr(Q < 32) return uint32_t(input_bits << (31 - Q));
        else return uint32_t(input_bits >> (Q - 31));
    }();

    auto iq29_result = fxmath::details::asin31(uiq31_input);
    #else
    
    const uint32_t uiq32_input = [&] -> uint32_t{
        return uint32_t(input_bits << (32 - Q));
    }();

    auto iq29_result = __IQNasin32(uiq32_input);
    #endif

    if(is_negative) iq29_result = -iq29_result;

    return iq29_result;
}
}

namespace ymd::math{

template<size_t Q>
__attribute__((const)) constexpr 
fixed<29, int32_t> asin(const fixed<Q, int32_t> x){
    return fxmath::details::asin32i(x);
}

template<size_t Q>
__attribute__((const)) constexpr 
fixed<29, int32_t> acos(const fixed<Q, int32_t> x){
    return fixed<29, int32_t>(M_PI/2) - fxmath::details::asin32i(x);
}

template<size_t Q>
__attribute__((const)) constexpr 
fixed<32, uint32_t> asinpu(const fixed<Q, int32_t> x){
    return rad_to_uq32(asin(x));
}

template<size_t Q>
__attribute__((const)) constexpr 
fixed<32, uint32_t> acospu(const fixed<Q, int32_t> x){
    return rad_to_uq32(acos(x));
}
}