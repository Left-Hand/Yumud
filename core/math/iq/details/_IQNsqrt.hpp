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

    template<size_t Q, const uint8_t type>
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

        #if 0
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
        #else
        if (uiq32Input < 0x40000000) {
            int leadingZeros = __builtin_clz(uiq32Input);
            int shiftsNeeded = leadingZeros / 2;  // 直接使用leadingZeros/2
            
            uiq32Input <<= 2 * shiftsNeeded;
            if constexpr(type != TYPE_ISQRT) {
                i16Exponent -= shiftsNeeded;
            } else {
                i16Exponent += shiftsNeeded;
            }
        }
        #endif

        return {
            uiq32Input,
            i16Exponent
        };
    }

    template<size_t Q, const uint8_t type>
    __attribute__((always_inline))
    static constexpr IqSqrtCoeffs from_u64 (uint64_t uiiqNInputX) {
        if(uiiqNInputX == 0) [[unlikely]]
            return {0, 0};

        uint32_t uiq32Input;
        int16_t i16Exponent;

        /* If the Q gives an odd starting exponent make it even. */
        if constexpr((32 - Q) % 2 == 1) {
            uiiqNInputX <<= 1;
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

        #if 0
        /* 将输入保存为无符号iq32 */
        /* 需要将64位输入适配到32位处理 */
        while ((uiiqNInputX >> 32)) {
            uiiqNInputX >>= 2;
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
        uiq32Input = uiiqNInputX;

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

        #else
        /* 将输入保存为无符号iq32 */
        /* 需要将64位输入适配到32位处理 - 使用高32位CLZ优化 */
        if (uiiqNInputX >> 32) {
            uint32_t high32 = (uint32_t)(uiiqNInputX >> 32);
            int clz_high = __builtin_clz(high32);          // 高32位前导零，范围0~31
            // 最高位位置 pos = 63 - clz_high (32~63)
            // 右移使pos ≤ 31，每次移2位，次数k = ceil((pos - 31)/2)
            // 推导: k = ceil((32 - clz_high)/2) = (33 - clz_high) >> 1
            int k = (33 - clz_high) >> 1;
            uiiqNInputX >>= (2 * k);
            if constexpr(type == TYPE_SQRT || type == TYPE_MAG) {
                i16Exponent += k;
            } else { // TYPE_ISQRT
                i16Exponent -= k;
            }
        }

        /* 保存为32位无符号数 */
        uiq32Input = (uint32_t)uiiqNInputX;

        /* 将32位输入规范化到 [0x40000000, 0xFFFFFFFF] - 使用CLZ优化 */
        if (uiq32Input != 0 && uiq32Input < 0x40000000) {
            int clz = __builtin_clz(uiq32Input);           // 32位前导零，非零时1~31
            // 最高位位置 p = 31 - clz (0~29)
            // 左移使p ≥ 30，每次移2位，次数k = ceil((30 - p)/2) = ceil((clz - 1)/2) = clz / 2
            int k = clz / 2;
            uiq32Input <<= (2 * k);
            if constexpr(type != TYPE_ISQRT) {
                i16Exponent -= k;
            } else {
                i16Exponent += k;
            }
        }
        #endif

        return {
            uiq32Input,
            i16Exponent
        };
    }


    template<size_t Q, const uint8_t type>
    __attribute__((always_inline))
    static constexpr IqSqrtCoeffs from_sqsum(uint64_t ui64Sum) {
        #if 0
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
        #else
        if (ui64Sum == 0) [[unlikely]]
            return {0, 0};

        int16_t i16Exponent;
        if constexpr (type == TYPE_MAG) {
            i16Exponent = (32 - int(Q));      // 幅度：指数初始为正
        } else {
            i16Exponent = (int(Q) - 32);      // 逆幅度：指数初始为负
        }

        /* ------------------------------------------------------------
        *  将64位平方和规范化，使高32位 ≥ 0x40000000。
        *  每次左移2位，同时调整指数。
        *  利用 __builtin_clz 一次计算所需移位次数，消除循环。
        * ------------------------------------------------------------
        */
        uint32_t high = static_cast<uint32_t>(ui64Sum >> 32);
        uint32_t low  = static_cast<uint32_t>(ui64Sum);

        int bit_pos;                          // 整个64位数中最高1的位置 (0 … 63)
        if (high != 0) {
            // 高32位非零：最高位位于高32位内
            bit_pos = 63 - __builtin_clz(high);
        } else {
            // 高32位为零，但整个数非零：最高位位于低32位内
            bit_pos = 31 - __builtin_clz(low);
        }

        // 目标：最高位移至第62位（高32位 ≥ 0x40000000 对应64位的第62位）
        // 需要左移的位数 = 62 - bit_pos，每次移2位 → 次数 k = ceil((62 - bit_pos)/2)
        int k = (62 - bit_pos + 1) / 2;       // 向上取整，且保证非负
        if (k > 0) {
            ui64Sum <<= (2 * k);             // 整体左移
            if constexpr (type == TYPE_MAG) {
                i16Exponent -= k;            // 左移使数值变大，幅度指数减小
            } else {
                i16Exponent += k;            // 逆幅度指数增大
            }
        }

        /* 此时高32位一定 ≥ 0x40000000，截取高32位作为iq32格式数值 */
        return {
            static_cast<uint32_t>(ui64Sum >> 32),
            i16Exponent
        };
        #endif

    }

    template<size_t Q, const uint8_t type>
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



template<size_t Q>
constexpr math::fixed_t<Q, uint32_t> _IQNsqrt(const math::fixed_t<Q, uint32_t> x){
    return math::fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_u32<Q, TYPE_SQRT>(
            x.to_bits()
        ).template compute<Q, TYPE_SQRT>()
    );
}


template<size_t Q>
constexpr math::fixed_t<Q, uint32_t> _IQNisqrt(const math::fixed_t<Q, uint32_t> x){
    return math::fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_u32<Q, TYPE_ISQRT>(
            x.to_bits()
        ).template compute<Q, TYPE_ISQRT>()
    );
}

template<size_t Q>
constexpr math::fixed_t<Q, uint32_t> _IQNsqrt64(const math::fixed_t<Q, uint64_t> x){
    return math::fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_u64<Q, TYPE_SQRT>(
            x.to_bits()
        ).template compute<Q, TYPE_SQRT>()
    );
}


template<size_t Q>
constexpr math::fixed_t<Q, uint32_t> _IQNisqrt64(const math::fixed_t<Q, uint64_t> x){
    return math::fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_u64<Q, TYPE_ISQRT>(
            x.to_bits()
        ).template compute<Q, TYPE_ISQRT>()
    );
}

// 计算单个值的平方（辅助函数）
template<typename T>
constexpr uint64_t square_value(const T& val) {
    using extended_t = std::conditional_t<std::is_signed_v<T>, int64_t, uint64_t>;
    auto bits = static_cast<extended_t>(val.to_bits());
    return static_cast<uint64_t>(bits * bits);
}

// 使用折叠表达式计算多个值的平方和
template<typename... Args>
requires (sizeof...(Args) > 0)
constexpr uint64_t sum_of_squares(Args&&... args) {

    return (square_value(args) + ...);

}

// 支持任意数量参数的模长计算
template<typename D, size_t Q, typename... Args>
constexpr math::fixed_t<Q, uint32_t> _IQNmag(math::fixed_t<Q, D> first, Args&&... rest) {
    uint64_t sum = sum_of_squares(first, rest...);
    return math::fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_sqsum<Q, TYPE_MAG>(sum).template compute<Q, TYPE_MAG>()
    );
}

// 支持任意数量参数的逆模长计算
template<typename D, size_t Q, typename... Args>
constexpr math::fixed_t<Q, uint32_t> _IQNimag(math::fixed_t<Q, D> first, Args&&... rest) {
    uint64_t sum = sum_of_squares(first, rest...);
    return math::fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_sqsum<Q, TYPE_IMAG>(sum).template compute<Q, TYPE_IMAG>()
    );
}


}

#undef TYPE_ISQRT
#undef TYPE_SQRT
#undef TYPE_MAG
#undef TYPE_IMAG