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

namespace ymd::iqmath::details{

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


struct IqSqrtCoeffs{
    uint32_t uiq32Input;
    int16_t i16Exponent;

    template<const size_t Q, const int32_t type>
    __attribute__((always_inline))
    static constexpr IqSqrtCoeffs from_u32(uint32_t iqNInputX) {
        if(iqNInputX == 0) [[unlikely]]
            return {0, 0};
        uint32_t uiq32Input;
        int16_t i16Exponent;

        /* If the Q gives an odd starting exponent make it even. */
        if constexpr((32 - Q) % 2 == 1) {
            iqNInputX <<= 1;
            /* Start with positive exponent for sqrt */
            if constexpr(type == TYPE_SQRT) {
                i16Exponent = ((32 - Q) - 1) >> 1;
            }
            /* start with negative exponent for isqrt */
            else {
                i16Exponent = -(((32 - Q) - 1) >> 1);
            }
        } else {
            /* start with positive exponent for sqrt */
            if constexpr(type == TYPE_SQRT) {
                i16Exponent = (32 - Q) >> 1;
            }
            /* start with negative exponent for isqrt */
            else {
                i16Exponent = -((32 - int(Q)) >> 1);
            }
        }

        /* Save input as unsigned iq32. */
        uiq32Input = iqNInputX;

        /* Shift to iq32 by keeping track of exponent */
        while ((uiq32Input) < 0x40000000) {
            uiq32Input <<= 2;
            /* Decrement exponent for sqrt and mag */
            if constexpr(type != TYPE_ISQRT) {
                i16Exponent--;
            }
            /* Increment exponent for isqrt */
            else {
                i16Exponent++;
            }
        }

        return {
            uiq32Input,
            i16Exponent
        };
    }

    template<const size_t Q, const int32_t type>
    __attribute__((always_inline))
    static constexpr IqSqrtCoeffs from_u64 (uint64_t iqNInputX) {
        if(iqNInputX == 0) [[unlikely]]
            return {0, 0};
        uint32_t uiq32Input;
        int16_t i16Exponent;

        /* If the Q gives an odd starting exponent make it even. */
        if constexpr((32 - Q) % 2 == 1) {
            iqNInputX <<= 1;
            /* Start with positive exponent for sqrt */
            if constexpr(type == TYPE_SQRT) {
                i16Exponent = ((32 - Q) - 1) >> 1;
            }
            /* start with negative exponent for isqrt */
            else {
                i16Exponent = -(((32 - Q) - 1) >> 1);
            }
        } else {
            /* start with positive exponent for sqrt */
            if constexpr(type == TYPE_SQRT) {
                i16Exponent = (32 - Q) >> 1;
            }
            /* start with negative exponent for isqrt */
            else {
                i16Exponent = -((32 - int(Q)) >> 1);
            }
        }

        /* 将输入保存为无符号iq32 */
        /* 需要将64位输入适配到32位处理 */
        while (iqNInputX > UINT32_MAX) {
            iqNInputX >>= 2;
            /* 对于sqrt和mag，递增指数 */
            if constexpr(type == TYPE_SQRT || type == TYPE_MAG) {
                i16Exponent++;
            }
            /* 对于isqrt，递减指数 */
            else {
                i16Exponent--;
            }
        }

        /* Save input as unsigned iq32. */
        uiq32Input = iqNInputX;

        /* Shift to iq32 by keeping track of exponent */
        while (((uiq32Input)) < 0x40000000) {
            uiq32Input <<= 2;
            /* Decrement exponent for sqrt and mag */
            if constexpr(type != TYPE_ISQRT) {
                i16Exponent--;
            }
            /* Increment exponent for isqrt */
            else {
                i16Exponent++;
            }
        }

        return {
            uiq32Input,
            i16Exponent
        };
    }


    template<const size_t Q, const int32_t type>
    __attribute__((always_inline))
    static constexpr IqSqrtCoeffs from_dual_u32(uint32_t iqNInputX, uint32_t iqNInputY) {
        /* Calculate y^2 and add to x^2 */
        uint64_t ui64Sum = (
            static_cast<uint64_t>(iqNInputX) * static_cast<uint64_t>(iqNInputX)
            + static_cast<uint64_t>(iqNInputY) * static_cast<uint64_t>(iqNInputY)
        );

        if(ui64Sum == 0) [[unlikely]]
            return {0, 0};
        int16_t i16Exponent;
        
        /*
        * Initialize the exponent to positive for magnitude, negative for
        * inverse magnitude.
        */
        if constexpr(type == TYPE_MAG) {
            i16Exponent = (32 - int(Q));
        } else {
            i16Exponent = (int(Q) - 32);
        }

        /* Shift to iq64 by keeping track of exponent. */
        while ((static_cast<uint32_t>(ui64Sum >> 32)) < 0x40000000) {
            ui64Sum <<= 2;
            if (type == TYPE_MAG) {
                /* Decrement exponent for mag */
                i16Exponent--;
            }else {
                /* Increment exponent for imag */
                i16Exponent++;
            }
        }

        /* Shift ui64Sum to unsigned iq32 and set as uiq32Input */
        return {
            (uint32_t)(ui64Sum >> 32),
            i16Exponent
        };
    }

    template<const size_t Q, const int32_t type>
    [[nodiscard]] constexpr uint32_t compute() && {
        if(uiq32Input == 0) [[unlikely]]
            return 0;
        uint32_t uiq30Guess;
        uint32_t uiq30Result;
        uint32_t uiq31Result;
        uint8_t ui8Index;

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
        
        /* Iterate through Newton-Raphson algorithm. */
        if constexpr(Q < 24) {
            IQNSQRT_ITER;
            IQNSQRT_ITER;
        } else {
            IQNSQRT_ITER;
            IQNSQRT_ITER;
            IQNSQRT_ITER;
        }

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
};



template<const size_t Q>
constexpr fixed_t<Q, uint32_t> _IQNsqrt(const fixed_t<Q, uint32_t> x){
    return fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_u32<Q, TYPE_SQRT>(
            x.to_bits()
        ).template compute<Q, TYPE_SQRT>()
    );
}


template<const size_t Q>
constexpr fixed_t<Q, uint32_t> _IQNisqrt(const fixed_t<Q, uint32_t> x){
    return fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_u32<Q, TYPE_ISQRT>(
            x.to_bits()
        ).template compute<Q, TYPE_ISQRT>()
    );
}

template<const size_t Q>
constexpr fixed_t<Q, uint32_t> _IQNsqrt64(const fixed_t<Q, uint64_t> x){
    return fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_u64<Q, TYPE_SQRT>(
            x.to_bits()
        ).template compute<Q, TYPE_SQRT>()
    );
}


template<const size_t Q>
constexpr fixed_t<Q, uint32_t> _IQNisqrt64(const fixed_t<Q, uint64_t> x){
    return fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_u64<Q, TYPE_ISQRT>(
            x.to_bits()
        ).template compute<Q, TYPE_ISQRT>()
    );
}

template<const size_t Q>
constexpr fixed_t<Q, uint32_t> _IQNmag(fixed_t<Q, uint32_t> x, fixed_t<Q, uint32_t> y){
    return fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_dual_u32<Q, TYPE_MAG>(
            x.to_bits(), y.to_bits()
        ).template compute<Q, TYPE_MAG>()
    );
}


template<const size_t Q>
constexpr fixed_t<Q, uint32_t> _IQNimag(fixed_t<Q, uint32_t> x, fixed_t<Q, uint32_t> y){
    return fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_dual_u32<Q, TYPE_IMAG>(
            x.to_bits(), y.to_bits()
        ).template compute<Q, TYPE_IMAG>()
    );
}

}

#undef TYPE_ISQRT
#undef TYPE_SQRT
#undef TYPE_MAG
#undef TYPE_IMAG