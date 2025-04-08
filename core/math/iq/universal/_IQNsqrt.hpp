#pragma once

#include "support.hpp"
#include "_IQNtables.hpp"



/*!
 * @brief Specifies inverse square root operation type.
 */
#define TYPE_ISQRT   (0)
/*!
 * @brief Specifies square root operation type.
 */
#define TYPE_SQRT    (1)
/*!
 * @brief Specifies magnitude operation type.
 */
#define TYPE_MAG     (2)
/*!
 * @brief Specifies inverse magnitude operation type.
 */
#define TYPE_IMAG    (3)

namespace __iqdetails{

/**
 * @brief Calculate square root, inverse square root and the magnitude of two inputs.
 *
 * @param iqNInputX         IQN type input x.
 * @param iqNInputY         IQN type input y.
 * @param type              Operation type.
 * @param Q           IQ format.
 *
 * @return                  IQN type result of the square root or magnitude operation.
 */
/*
 * Calculate square root, inverse square root and the magnitude of two inputs
 * using a Newton-Raphson iterative method. This method takes an initial guess
 * and performs an error correction with each iteration. The equation is:
 *
 *     x1 = x0 - f(x0)/f'(x0)
 *
 * Where f' is the derivative of f. The approximation for inverse square root
 * is:
 *
 *     g' = g * (1.5 - (x/2) * g * g)
 *
 *     g' = new guess approximation
 *     g = best guess approximation
 *     x = input
 *
 * The inverse square root is multiplied by the initial input x to get the
 * square root result for square root and magnitude functions.
 *
 *     root(x) = x * 1/root(x)
 */

template<const int Q, const int32_t type>
constexpr int32_t __IQNsqrt(int32_t iqNInputX, int32_t iqNInputY){
    uint8_t ui8Index;
    int16_t i16Exponent;
    uint32_t uiq30Guess;
    uint32_t uiq30Result;
    uint32_t uiq31Result;
    uint32_t uiq32Input;

    /* If the type is (inverse) magnitude we need to calculate x^2 + y^2 first. */
    if constexpr(type == TYPE_MAG || type == TYPE_IMAG) {
        uint64_t ui64Sum;

        /* Calculate x^2 */
        ui64Sum = __mpyx(iqNInputX, iqNInputX);

        /* Calculate y^2 and add to x^2 */
        ui64Sum += __mpyx(iqNInputY, iqNInputY);

        /* Return if the magnitude is simply zero. */
        if (ui64Sum == 0) {
            return 0;
        }

        /*
         * Initialize the exponent to positive for magnitude, negative for
         * inverse magnitude.
         */
        if constexpr(type == TYPE_MAG) {
            i16Exponent = (32 - Q);
        } else {
            i16Exponent = (Q - 32);
        }

        /* Shift to iq64 by keeping track of exponent. */
        while ((uint16_t)(ui64Sum >> 48) < 0x4000) {
            ui64Sum <<= 2;
            /* Decrement exponent for mag */
            if (type == TYPE_MAG) {
                i16Exponent--;
            }
            /* Increment exponent for imag */
            else {
                i16Exponent++;
            }
        }

        /* Shift ui64Sum to unsigned iq32 and set as uiq32Input */
        uiq32Input = (uint32_t)(ui64Sum >> 32);
    } else {
        /* check sign of input */
        if (iqNInputX <= 0) {
            return 0;
        }

        /* If the Q gives an odd starting exponent make it even. */
        if ((32 - Q) % 2 == 1) {
            iqNInputX <<= 1;
            /* Start with positive exponent for sqrt */
            if (type == TYPE_SQRT) {
                i16Exponent = ((32 - Q) - 1) >> 1;
            }
            /* start with negative exponent for isqrt */
            else {
                i16Exponent = -(((32 - Q) - 1) >> 1);
            }
        } else {
            /* start with positive exponent for sqrt */
            if (type == TYPE_SQRT) {
                i16Exponent = (32 - Q) >> 1;
            }
            /* start with negative exponent for isqrt */
            else {
                i16Exponent = -((32 - Q) >> 1);
            }
        }

        /* Save input as unsigned iq32. */
        uiq32Input = (uint32_t)iqNInputX;

        /* Shift to iq32 by keeping track of exponent */
        while ((uint16_t)(uiq32Input >> 16) < 0x4000) {
            uiq32Input <<= 2;
            /* Decrement exponent for sqrt and mag */
            if (type) {
                i16Exponent--;
            }
            /* Increment exponent for isqrt */
            else {
                i16Exponent++;
            }
        }
    }


    /* Use left most byte as index into lookup table (range: 32-128) */
    ui8Index = uiq32Input >> 25;
    ui8Index -= 32;
    uiq30Guess = (uint32_t)_IQ14sqrt_lookup[ui8Index] << 16;

    /*
     * Set the loop counter:
     *
     *     iq1 <= Q < 24 - 2 loops
     *     iq22 <= Q <= 31 - 3 loops
     */

    #define IQNSQRT_ITER\
    uiq31Result = __mpyf_ul(uiq32Input, uiq30Guess);\
    uiq30Result = __mpyf_ul(uiq31Result, uiq30Guess);\
    uiq30Result = -(uiq30Result - 0xC0000000);\
    uiq30Guess = __mpyf_ul(uiq30Guess, uiq30Result);\
    
    if constexpr(Q < 24) {
        IQNSQRT_ITER;
        IQNSQRT_ITER;
    } else {
        IQNSQRT_ITER;
        IQNSQRT_ITER;
        IQNSQRT_ITER;
    }

    /* Iterate through Newton-Raphson algorithm. */
    // while (ui8Loops--) {
    //     /* x*g */
    //     uiq31Result = __mpyf_ul(uiq32Input, uiq30Guess);

    //     /* x*g*g */
    //     uiq30Result = __mpyf_ul(uiq31Result, uiq30Guess);

    //     /* 3 - x*g*g */
    //     uiq30Result = -(uiq30Result - 0xC0000000);

    //     /*
    //      * g/2*(3 - x*g*g)
    //      * uiq30Guess = uiq31Guess/2
    //      */
    //     uiq30Guess = __mpyf_ul(uiq30Guess, uiq30Result);
    // }

    /* Calculate sqrt(x) for both sqrt and mag */
    if constexpr(type == TYPE_SQRT || type == TYPE_MAG) {
        /*
         * uiq30Guess contains the inverse square root approximation, multiply
         * by uiq32Input to get square root result.
         */
        uiq31Result = __mpyf_ul(uiq30Guess, uiq32Input);


        /*
         * Shift the result right by 31 - Q.
         */
        i16Exponent -= (31 - Q);

        /* Saturate value for any shift larger than 1 (only need this for mag) */
        if constexpr(type == TYPE_MAG) {
            if (i16Exponent > 0) {
                return 0x7fffffff;
            }
        }

        /* Shift left by 1 check only needed for iq30 and iq31 mag/sqrt */
        if constexpr(Q >= 30) {
            if (i16Exponent > 0) {
                uiq31Result <<= 1;
                return uiq31Result;
            }
        }
    }
    /* Separate handling for isqrt and imag. */
    else {

        /*
         * Shift the result right by 31 - Q, add one since we use the uiq30
         * result without shifting.
         */
        i16Exponent = i16Exponent - (31 - Q) + 1;
        uiq31Result = uiq30Guess;

        /* Saturate any positive non-zero exponent for isqrt. */
        if (i16Exponent > 0) {
            return 0x7fffffff;
        }
    }

    /* Shift uiq31Result right by -exponent */
    if (i16Exponent <= -32) {
        return 0;
    }
    if (i16Exponent <= -16) {
        uiq31Result >>= 16;
        i16Exponent += 16;
    }
    if (i16Exponent <= -8) {
        uiq31Result >>= 8;
        i16Exponent += 8;
    }
    while (i16Exponent < -1) {
        uiq31Result >>= 1;
        i16Exponent++;
    }
    if (i16Exponent) {
        uiq31Result++;
        uiq31Result >>= 1;
    }

    return uiq31Result;
}

template<const size_t Q>
constexpr _iq<Q> _IQNsqrt(const _iq<Q> iqNInputX){
    return std::bit_cast<_iq<Q>>(
        __IQNsqrt<Q, TYPE_SQRT>(
            std::bit_cast<int32_t>(iqNInputX), 
            std::bit_cast<int32_t>(0)
        )
    );
}


template<const size_t Q>
constexpr _iq<Q> _IQNisqrt(const _iq<Q> iqNInputX){
    return std::bit_cast<_iq<Q>>(
        __IQNsqrt<Q, TYPE_ISQRT>(
            std::bit_cast<int32_t>(iqNInputX), 
            std::bit_cast<int32_t>(0)
        )
    );
}


template<const size_t Q>
constexpr _iq<Q> _IQNmag(_iq<Q> iqNInputX, _iq<Q> iqNInputY){
    return std::bit_cast<_iq<Q>>(
        __IQNsqrt<Q, TYPE_MAG>(
            std::bit_cast<int32_t>(iqNInputX), 
            std::bit_cast<int32_t>(iqNInputY)
        )
    );
}


template<const size_t Q>
constexpr _iq<Q> _IQNimag(_iq<Q> iqNInputX, _iq<Q> iqNInputY){
    return std::bit_cast<_iq<Q>>(
        __IQNsqrt<Q, TYPE_IMAG>(
            std::bit_cast<int32_t>(iqNInputX), 
            std::bit_cast<int32_t>(iqNInputY)
        )
    );
}

}

#undef TYPE_ISQRT
#undef TYPE_SQRT
#undef TYPE_MAG
#undef TYPE_IMAG