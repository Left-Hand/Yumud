#pragma once

#include "support.hpp"
#include "_IQNtables.hpp"

#include "_IQNsqrt.hpp"

namespace ymd::iqmath::details{
/**
 * @brief Computes the inverse sine of the IQN input.
 *
 * @param iqNInput        IQN type input.
 * @param Q         IQ format.
 *
 * @return                IQN type result of inverse sine.
 */
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


constexpr math::fixed<29, int32_t> __IQNasin31(uint32_t uiq31Input)
{
    bool is_acos = false;

    /*
     * Apply the transformation from asin to acos if input is greater than 0.5.
     * The first step is to calculate the following:
     *
     *     (sqrt((1 - uiq31Input)/2))
     */
    uint32_t uiq32Input;
    {
        const uint32_t Temp = 0x80000000u - uiq31Input;
        if (Temp < 0x40000000u) {
            /* Calculate sqrt((1 - uiq31Input)/2) */
            #if 0
            // uiq32Input = _IQNsqrt32<31>(math::fixed<31, uint32_t>::from_bits(Temp >> 1)).to_bits() << 1;
            #else
            // uiq32Input = _IQNsqrt32<31>(math::fixed<31, uint32_t>::from_bits(Temp >> 1)).to_bits() << 1;
            // uiq32Input = _IQNsqrt32<31>(math::fixed<32, uint32_t>::from_bits(Temp)).to_bits();
            uiq32Input = _IQNsqrt32<32>(math::fixed<32, uint32_t>::from_bits(Temp << 1)).to_bits();
            // uiq32Input = _IQNsqrt32<31>(math::fixed<31, uint32_t>::from_bits(Temp >> 1)).to_bits() << 1;
            #endif

            /* Flag that the transformation was used. */
            is_acos = true;
        }else{
            uiq32Input = uiq31Input << 1;
            is_acos = false;
        }
    }
    
    /* Calculate the index using the left 6 most bits of the input. */
    /* Set the coefficient pointer. */
    const int32_t * piq29Coeffs = IQ29ASIN_COEFFS[size_t((uiq32Input >> 27) & 0x003f)];
    int32_t iq29Result;

    /*
     * Calculate asin(x) using the following Taylor series:
     *
     *     asin(x) = (((c4*x + c3)*x + c2)*x + c1)*x + c0
     */

    /* c4*x */
    iq29Result = int32_t(((int64_t(uiq32Input) * (piq29Coeffs[0])) >> 32));

    /* c4*x + c3 */
    iq29Result += piq29Coeffs[1];

    /* (c4*x + c3)*x */
    iq29Result = int32_t(((int64_t(uiq32Input) * iq29Result) >> 32));

    /* (c4*x + c3)*x + c2 */
    iq29Result += piq29Coeffs[2];

    /* ((c4*x + c3)*x + c2)*x */
    iq29Result = int32_t(((int64_t(uiq32Input) * iq29Result) >> 32));

    /* ((c4*x + c3)*x + c2)*x + c1 */
    iq29Result += piq29Coeffs[3];

    /* (((c4*x + c3)*x + c2)*x + c1)*x */
    iq29Result = int32_t(((int64_t(uiq32Input) * iq29Result) >> 32));

    /* (((c4*x + c3)*x + c2)*x + c1)*x + c0 */
    iq29Result += piq29Coeffs[4];

    /* check if we switched to acos */
    if (is_acos) {
        /* asin(x) = pi/2 - 2*iq29Result */

        // acos(x) = -(2 * asin(sqrt(1-x)/2)-pi/2)
        // 2\arcsin\left(\sqrt{\frac{\left(1-x\right)}{2}}\right)-\frac{\pi}{2}
        iq29Result = iq29Result << 1;
        iq29Result -= _iq29_halfPi;      // this is equivalent to the above
        iq29Result = -iq29Result;       // but avoids using temporary registers
    }

    return math::fixed<29, int32_t>::from_bits(iq29Result);

}


constexpr math::fixed<29, int32_t> __IQNasin32(uint32_t uiq32Input)
{
    bool is_acos = false;

    /*
     * Apply the transformation from asin to acos if input is greater than 0.5.
     * The first step is to calculate the following:
     *
     *     (sqrt((1 - x)/2))
     */
    {
        const uint32_t Temp = ~uiq32Input;
        if (Temp < 0x80000000u) {
            // uiq32Input = _IQNsqrt32<32>(math::fixed<32, uint32_t>::from_bits(Temp)).to_bits();
            uiq32Input = _IQNsqrt32<32>(math::fixed<32, uint32_t>::from_bits(Temp)).to_bits() >> 1;
            is_acos = true;
        }else{
            is_acos = false;
        }
    }
    
    /* Calculate the index using the left 6 most bits of the input. */
    /* Set the coefficient pointer. */
    // const int32_t * piq29Coeffs = IQ29ASIN_COEFFS[size_t((uiq32Input >> 27) & 0x003f)];
    const int32_t * piq29Coeffs = IQ29ASIN_COEFFS[size_t((uiq32Input >> 27))];
    int32_t iq29Result;

    /*
     * Calculate asin(x) using the following Taylor series:
     *
     *     asin(x) = (((c4*x + c3)*x + c2)*x + c1)*x + c0
     */

    /* c4*x */
    iq29Result = int32_t(((int64_t(uiq32Input) * (*piq29Coeffs++)) >> 32));

    /* c4*x + c3 */
    iq29Result += *piq29Coeffs++;

    /* (c4*x + c3)*x */
    iq29Result = int32_t(((int64_t(uiq32Input) * iq29Result) >> 32));

    /* (c4*x + c3)*x + c2 */
    iq29Result += *piq29Coeffs++;

    /* ((c4*x + c3)*x + c2)*x */
    iq29Result = int32_t(((int64_t(uiq32Input) * iq29Result) >> 32));

    /* ((c4*x + c3)*x + c2)*x + c1 */
    iq29Result += *piq29Coeffs++;

    /* (((c4*x + c3)*x + c2)*x + c1)*x */
    iq29Result = int32_t(((int64_t(uiq32Input) * iq29Result) >> 32));

    /* (((c4*x + c3)*x + c2)*x + c1)*x + c0 */
    iq29Result += *piq29Coeffs++;

    /* check if we switched to acos */
    if (is_acos) {
        /* asin(x) = pi/2 - 2*iq29Result */

        // acos(x) = -(2 * asin(sqrt(1-x)/2)-pi/2)
        // 2\arcsin\left(\sqrt{\frac{\left(1-x\right)}{2}}\right)-\frac{\pi}{2}
        iq29Result = iq29Result << 1;
        iq29Result -= _iq29_halfPi;      // this is equivalent to the above
        iq29Result = -iq29Result;       // but avoids using temporary registers
    }

    return math::fixed<29, int32_t>::from_bits(iq29Result);

}

template<const size_t Q>
constexpr math::fixed<29, int32_t> _IQNasin(math::fixed<Q, int32_t> iqNInput){
    static_assert(Q <= 32);
    uint32_t input_bits = std::bit_cast<uint32_t>(iqNInput.to_bits());
    const bool is_neg  = input_bits & (1u << 31);
    if(is_neg) input_bits = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(input_bits));

    if constexpr(Q < 32)
        if(input_bits > (uint32_t(1) << Q)) [[unlikely]]{
            input_bits = uint32_t(1) << Q;
        }

    #if 1
    const uint32_t uiq31Input = [&] -> uint32_t{
        if constexpr(Q < 32) return uint32_t(input_bits << (31 - Q));
        else return uint32_t(input_bits >> 1);
    }();

    auto iq29_result = __IQNasin31(uiq31Input);
    #else
    
    const uint32_t uiq32Input = [&] -> uint32_t{
        return uint32_t(input_bits << (32 - Q));
    }();

    auto iq29_result = __IQNasin32(uiq32Input);
    #endif

    if(is_neg) iq29_result = -iq29_result;

    return iq29_result;
}



}