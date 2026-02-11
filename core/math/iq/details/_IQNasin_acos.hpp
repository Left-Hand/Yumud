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


constexpr int32_t __IQNasin(uint32_t uiq31Input)
{

    
    bool is_acos;

    /*
     * Apply the transformation from asin to acos if input is greater than 0.5.
     * The first step is to calculate the following:
     *
     *     (sqrt((1 - uiq31Input)/2))
     */
    {
        const uint32_t uiq31InputTemp = 0x80000000u - uiq31Input;
        if (uiq31InputTemp < 0x40000000u) {
            /* Halve the result. */
            uiq31Input = uiq31InputTemp >> 1;

            /* Calculate sqrt((1 - uiq31Input)/2) */
            uiq31Input = _IQNsqrt<31>(math::fixed_t<31, int32_t>::from_bits(uiq31Input)).to_bits();

            /* Flag that the transformation was used. */
            is_acos = true;
        }else{
            is_acos = false;
        }
    }

    [[maybe_unused]] const uint32_t uiq32Input = (uiq31Input << 1); 

    
    /* Calculate the index using the left 6 most bits of the input. */
    /* Set the coefficient pointer. */
    const int32_t * piq29Coeffs = _IQ29Asin_coeffs[(uint16_t)(uiq31Input >> 26) & 0x003f];
    int32_t iq29Result;

    /*
     * Calculate asin(x) using the following Taylor series:
     *
     *     asin(x) = (((c4*x + c3)*x + c2)*x + c1)*x + c0
     */

    /* c4*x */
    iq29Result = int32_t(((int64_t(uiq32Input) * (*piq29Coeffs++)) >> 32));

    /* c4*x + c3 */
    iq29Result = iq29Result + *piq29Coeffs++;

    /* (c4*x + c3)*x */
    iq29Result = int32_t(((int64_t(uiq32Input) * iq29Result) >> 32));

    /* (c4*x + c3)*x + c2 */
    iq29Result = iq29Result + *piq29Coeffs++;

    /* ((c4*x + c3)*x + c2)*x */
    iq29Result = int32_t(((int64_t(uiq32Input) * iq29Result) >> 32));

    /* ((c4*x + c3)*x + c2)*x + c1 */
    iq29Result = iq29Result + *piq29Coeffs++;

    /* (((c4*x + c3)*x + c2)*x + c1)*x */
    iq29Result = int32_t(((int64_t(uiq32Input) * iq29Result) >> 32));

    /* (((c4*x + c3)*x + c2)*x + c1)*x + c0 */
    iq29Result = iq29Result + *piq29Coeffs++;

    /* check if we switched to acos */
    if (is_acos) {
        /* asin(x) = pi/2 - 2*iq29Result */
        iq29Result = iq29Result << 1;
        iq29Result -= _iq29_halfPi;      // this is equivalent to the above
        iq29Result = -iq29Result;       // but avoids using temporary registers
    }

    return iq29Result;

}

template<const size_t Q>
constexpr math::fixed_t<29, int32_t> _IQNasin(math::fixed_t<Q, int32_t> iqNInput){
    uint32_t input_bits = std::bit_cast<uint32_t>(iqNInput.to_bits());
    const bool is_neg  = input_bits & (1u << 31);
    if(is_neg) input_bits = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(input_bits));

    if constexpr(Q < 32)
        if(input_bits > (uint32_t(1) << Q)) 
            return 0;

    const uint32_t uiq31Input = [&] -> uint32_t{
        if constexpr(Q < 32) return (uint32_t)input_bits << (31 - Q);
        else return (input_bits >> 1);
    }();

    int32_t iq29_ret_bits = __IQNasin(uiq31Input);


    if(is_neg) iq29_ret_bits = -iq29_ret_bits;

    return math::fixed_t<29, int32_t>::from_bits(iq29_ret_bits);
}



}