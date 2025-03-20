#pragma once

#include "support.hpp"
#include "_IQNtables.hpp"

#include "_IQNsqrt.hpp"

namespace __iqdetails{
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

template<const size_t Q>
constexpr int32_t __IQNasin(int32_t iqNInput)
{
    uint8_t ui8Status = 0;
    uint16_t index;
    int32_t iq29Result;
    const int32_t *piq29Coeffs;
    uint32_t uiq31Input;
    uint32_t uiq31InputTemp;

    /*
     * Extract the sign from the input and set the following status bits:
     *
     *      ui8Status = xxxxxxTS
     *      x = unused
     *      T = transform was applied
     *      S = sign bit needs to be set (-y)
     */
    if (iqNInput < 0) {
        ui8Status |= 1;
        iqNInput = -iqNInput;
    }

    /*
     * Check if input is within the valid input range:
     *
     *     0 <= iqNInput <= 1
     */
    if (iqNInput > ((int32_t)1 << Q)) {
        return 0;
    }

    /* Convert input to unsigned iq31. */
    uiq31Input = (uint32_t)iqNInput << (31 - Q);

    /*
     * Apply the transformation from asin to acos if input is greater than 0.5.
     * The first step is to calculate the following:
     *
     *     (sqrt((1 - uiq31Input)/2))
     */
    uiq31InputTemp = 0x80000000 - uiq31Input;
    if (uiq31InputTemp < 0x40000000) {
        /* Halve the result. */
        uiq31Input = uiq31InputTemp >> 1;

        /* Calculate sqrt((1 - uiq31Input)/2) */
        uiq31Input = std::bit_cast<int32_t>(_IQNsqrt<31>(std::bit_cast<_iq<31>>(uiq31Input)));

        /* Flag that the transformation was used. */
        ui8Status |= 2;
    }

    /* Calculate the index using the left 6 most bits of the input. */
    index = (int16_t)(uiq31Input >> 26) & 0x003f;

    /* Set the coefficient pointer. */
    piq29Coeffs = _IQ29Asin_coeffs[index];


    /*
     * Calculate asin(x) using the following Taylor series:
     *
     *     asin(x) = (((c4*x + c3)*x + c2)*x + c1)*x + c0
     */

    /* c4*x */
    iq29Result = __mpyf_l(uiq31Input, *piq29Coeffs++);

    /* c4*x + c3 */
    iq29Result = iq29Result + *piq29Coeffs++;

    /* (c4*x + c3)*x */
    iq29Result = __mpyf_l(uiq31Input, iq29Result);

    /* (c4*x + c3)*x + c2 */
    iq29Result = iq29Result + *piq29Coeffs++;

    /* ((c4*x + c3)*x + c2)*x */
    iq29Result = __mpyf_l(uiq31Input, iq29Result);

    /* ((c4*x + c3)*x + c2)*x + c1 */
    iq29Result = iq29Result + *piq29Coeffs++;

    /* (((c4*x + c3)*x + c2)*x + c1)*x */
    iq29Result = __mpyf_l(uiq31Input, iq29Result);

    /* (((c4*x + c3)*x + c2)*x + c1)*x + c0 */
    iq29Result = iq29Result + *piq29Coeffs++;


    /* check if we switched to acos */
    if (ui8Status & 2) {
        /* asin(x) = pi/2 - 2*iq29Result */
        iq29Result = iq29Result << 1;
        iq29Result -= _iq29_halfPi;      // this is equivalent to the above
        iq29Result = -iq29Result;       // but avoids using temporary registers
    }
    /* Add sign to the result. */
    if (ui8Status & 1) {
        return (-iq29Result);
    }else{
        return (iq29Result);
    }
}


template<const size_t Q>
constexpr _iq<29> _IQNasin(_iq<Q> iqNInput){
    // static_assert(Q <= 29, "Input must be 29 bits or less.");

    if constexpr (Q > 29){
        return std::bit_cast<_iq<29>>(
            _IQNasin<29>(_iq<29>(iqNInput).to_i32())
        );
    }else{
        return std::bit_cast<_iq<29>>(
            __IQNasin<Q>(iqNInput.to_i32())
        );
    }
}

}