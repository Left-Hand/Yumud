#pragma once

#include "support.hpp"
#include "_IQNtables.hpp"





namespace ymd::iqmath::details{
enum class [[nodiscard]] SqrtNormStrategy {
    SQRT,   // from_u64 with SqrtNormStrategy::SQRT
    ISQRT,  // from_u64 with SqrtNormStrategy::ISQRT
    MAG,    // from_sqsum with SqrtNormStrategy::MAG
    IMAG    // from_sqsum with SqrtNormStrategy::IMAG
};


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


struct alignas(8) [[nodiscard]] IqSqrtCoeffs final{
    uint32_t uiq32Input;
    int32_t i16Exponent;

    template<size_t Q, const SqrtNormStrategy STRATEGY>
    __attribute__((always_inline))
    static constexpr IqSqrtCoeffs from_u32(uint32_t iqNInputX) {
        if(iqNInputX == 0) [[unlikely]]
            return {0, 0};
        uint32_t uiq32Input;
        int32_t i16Exponent;

        /* If the Q gives an odd starting exponent make it even. */
        if constexpr((32 - Q) % 2 == 1) {
            iqNInputX <<= 1;
            /* Start with positive exponent for sqrt */
            if constexpr(STRATEGY == SqrtNormStrategy::SQRT) {
                i16Exponent = ((32 - Q) - 1) >> 1;
            }
            /* start with negative exponent for isqrt */
            else {
                i16Exponent = -(((32 - Q) - 1) >> 1);
            }
        } else {
            /* start with positive exponent for sqrt */
            if constexpr(STRATEGY == SqrtNormStrategy::SQRT) {
                i16Exponent = (32 - Q) >> 1;
            }
            /* start with negative exponent for isqrt */
            else {
                i16Exponent = -((32 - int32_t(Q)) >> 1);
            }
        }

        /* Save input as unsigned iq32. */
        uiq32Input = iqNInputX;

        if (uiq32Input < 0x40000000) [[likely]] {
            int32_t leadingZeros = __builtin_clz(uiq32Input);
            int32_t shiftsNeeded = leadingZeros / 2;  // 直接使用leadingZeros/2
            
            uiq32Input <<= 2 * shiftsNeeded;
            if constexpr(STRATEGY != SqrtNormStrategy::ISQRT) {
                i16Exponent -= shiftsNeeded;
            } else {
                i16Exponent += shiftsNeeded;
            }
        }

        return {
            uiq32Input,
            i16Exponent
        };
    }


    template<size_t Q, const SqrtNormStrategy STRATEGY>
    [[nodiscard]] constexpr uint32_t compute() && {
        if(uiq32Input == 0) [[unlikely]]
            return 0;

        uint32_t uiq30Guess;
        uint32_t uiq30Result;
        uint32_t uiq31Result;

        /* Use left most byte as index into lookup table (range: 32-128) */
        const uint32_t ui8Index = ((uiq32Input >> 25) - 32);
        uiq30Guess = (uint32_t)_IQ14sqrt_lookup[ui8Index] << 16;

        /*
        * Set the loop counter:
        *
        *     iq1 <= Q < 24 - 2 loops
        *     iq22 <= Q <= 31 - 3 loops
        */

        auto newton_iter = [&]() __attribute__((always_inline)){
            uiq31Result = __mpyf_ul(uiq32Input, uiq30Guess);
            uiq30Result = __mpyf_ul(uiq31Result, uiq30Guess);
            uiq30Result = -(uiq30Result - 0xC0000000);
            uiq30Guess = __mpyf_ul(uiq30Guess, uiq30Result);
        };

        /* Iterate through Newton-Raphson algorithm. */
        newton_iter();
        newton_iter();
        
        if constexpr (Q >= 24) {
            newton_iter();
        }

        /* Calculate sqrt(x) for both sqrt and mag */
        if constexpr(STRATEGY == SqrtNormStrategy::SQRT || STRATEGY == SqrtNormStrategy::MAG) {
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
            if constexpr(STRATEGY == SqrtNormStrategy::MAG) {
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


        /* Shift uiq31Result right by -exponent —— 使用 clz 优化 */
        int32_t shift = -i16Exponent;
        if (shift <= 0) {
            return uiq31Result;
        }

        /* 若结果为 0 则直接返回（防御性编程） */
        if (uiq31Result == 0) [[unlikely]] {
            return 0;
        }

        /* 利用 clz 获取有效位宽，若移位量 ≥ 位宽则结果必为 0 */
        int32_t bits = 32 - __builtin_clz(uiq31Result);
        if (shift >= bits) {
            return 0;
        }

        /* 一次性右移，并保持原舍入逻辑：最后 1 位做加 1 后右移，之前各位无舍入 */
        if (shift > 1) {
            uiq31Result >>= (shift - 1);
            uiq31Result = (uiq31Result + 1) >> 1;
        } else {  /* shift == 1 */
            uiq31Result = (uiq31Result + 1) >> 1;
        }

        return uiq31Result;
    }

    template<size_t Q, const SqrtNormStrategy STRATEGY>
    __attribute__((always_inline))
    static constexpr IqSqrtCoeffs from_u64(uint64_t uiiqNInputX) {
        if (uiiqNInputX == 0) [[unlikely]]
            return {0, 0};

        int32_t i16Exponent;

        // ---------- 1. 奇偶校正与指数初始化（完全保留原逻辑）----------
        if constexpr ((32 - Q) % 2 == 1) {
            uiiqNInputX <<= 1;
            if constexpr (STRATEGY == SqrtNormStrategy::SQRT) {
                i16Exponent = ((32 - Q) - 1) >> 1;
            } else {   // 包含 ISQRT 和 MAG（原代码如此，保持原样）
                i16Exponent = -(((32 - Q) - 1) >> 1);
            }
        } else {
            if constexpr (STRATEGY == SqrtNormStrategy::SQRT) {
                i16Exponent = (32 - Q) >> 1;
            } else {
                i16Exponent = -((32 - int32_t(Q)) >> 1);
            }
        }

        // ---------- 2. 仿照 from_sqnum：一次 clz 得到最高位位置 ----------
        uint32_t high = static_cast<uint32_t>(uiiqNInputX >> 32);
        uint32_t low  = static_cast<uint32_t>(uiiqNInputX);

        int32_t bit_pos;   // 当前 64 位值中最高 1 的位置 (0 … 63)
        if (high != 0) [[likely]] {
            bit_pos = 63 - __builtin_clz(high);
        } else {
            bit_pos = 31 - __builtin_clz(low);   // low 一定非零（已排除 0）
        }

        // ---------- 3. 目标最高位位置：与 bit_pos 同奇偶，30 或 31 ----------
        int32_t target = 30 + (bit_pos & 1);

        // ---------- 4. 计算偶数移位量，一次性完成规范化 ----------
        int32_t shift = bit_pos - target;   // 正数：右移；负数：左移
        int32_t k;                         // 移 2 位的次数
        if (shift >= 0) {
            uiiqNInputX >>= shift;
            k = shift >> 1;
            if constexpr (STRATEGY == SqrtNormStrategy::SQRT || 
                        STRATEGY == SqrtNormStrategy::MAG) {
                i16Exponent += k;      // 右移 → 数值变小 → 指数增大
            } else {                  // ISQRT
                i16Exponent -= k;      // 右移 → 逆幅度数值变大 → 指数减小
            }
        } else {
            uiiqNInputX <<= -shift;
            k = (-shift) >> 1;
            if constexpr (STRATEGY == SqrtNormStrategy::SQRT || 
                        STRATEGY == SqrtNormStrategy::MAG) {
                i16Exponent -= k;      // 左移 → 数值变大 → 指数减小
            } else {                  // ISQRT
                i16Exponent += k;      // 左移 → 逆幅度数值变小 → 指数增大
            }
        }

        // ---------- 5. 低 32 位即为规范化后的 iq32 数值 ----------
        uint32_t uiq32Input = static_cast<uint32_t>(uiiqNInputX);
        // 此时 uiq32Input 一定 ≥ 0x40000000（可加断言）
        return { uiq32Input, i16Exponent };
    }

    template<size_t Q, const SqrtNormStrategy STRATEGY>
    __attribute__((always_inline))
    static constexpr IqSqrtCoeffs from_sqsum(uint64_t ui64Sum) {

        if (ui64Sum == 0) [[unlikely]]
            return {0, 0};

        int32_t i16Exponent;
        if constexpr (STRATEGY == SqrtNormStrategy::MAG) {
            i16Exponent = (32 - int32_t(Q));      // 幅度：指数初始为正
        } else {
            i16Exponent = (int32_t(Q) - 32);      // 逆幅度：指数初始为负
        }

        /* ------------------------------------------------------------
        *  将64位平方和规范化，使高32位 ≥ 0x40000000。
        *  每次左移2位，同时调整指数。
        *  利用 __builtin_clz 一次计算所需移位次数，消除循环。
        * ------------------------------------------------------------
        */
        uint32_t high = static_cast<uint32_t>(ui64Sum >> 32);
        uint32_t low  = static_cast<uint32_t>(ui64Sum);

        int32_t bit_pos;                          // 整个64位数中最高1的位置 (0 … 63)
        if (high != 0) [[likely]] {
            // 高32位非零：最高位位于高32位内
            bit_pos = 63 - __builtin_clz(high);
        } else {
            // 高32位为零，但整个数非零：最高位位于低32位内
            bit_pos = 31 - __builtin_clz(low);
        }

        // 目标：最高位移至第62位（高32位 ≥ 0x40000000 对应64位的第62位）
        // 需要左移的位数 = 62 - bit_pos，每次移2位 → 次数 k = ceil((62 - bit_pos)/2)
        int32_t k = (62 - bit_pos + 1) / 2;       // 向上取整，且保证非负
        if (k > 0) {
            ui64Sum <<= (2 * k);             // 整体左移
            if constexpr (STRATEGY == SqrtNormStrategy::MAG) {
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

    }

private:
};



template<size_t Q>
constexpr math::fixed_t<Q, uint32_t> _IQNsqrt32(const math::fixed_t<Q, uint32_t> x){
    return math::fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_u32<Q, SqrtNormStrategy::SQRT>(
            x.to_bits()
        ).template compute<Q, SqrtNormStrategy::SQRT>()
    );
}


template<size_t Q>
constexpr math::fixed_t<Q, uint32_t> _IQNisqrt32(const math::fixed_t<Q, uint32_t> x){
    return math::fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_u32<Q, SqrtNormStrategy::ISQRT>(
            x.to_bits()
        ).template compute<Q, SqrtNormStrategy::ISQRT>()
    );
}

template<size_t Q>
constexpr math::fixed_t<Q, uint32_t> _IQNsqrt64(const math::fixed_t<Q, uint64_t> x){
    return math::fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_u64<Q, SqrtNormStrategy::SQRT>(
            x.to_bits()
        ).template compute<Q, SqrtNormStrategy::SQRT>()
    );
}


template<size_t Q>
constexpr math::fixed_t<Q, uint32_t> _IQNisqrt64(const math::fixed_t<Q, uint64_t> x){
    return math::fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_u64<Q, SqrtNormStrategy::ISQRT>(
            x.to_bits()
        ).template compute<Q, SqrtNormStrategy::ISQRT>()
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
        IqSqrtCoeffs::template from_sqsum<Q, SqrtNormStrategy::MAG>(sum).template compute<Q, SqrtNormStrategy::MAG>()
    );
}

// 支持任意数量参数的逆模长计算
template<typename D, size_t Q, typename... Args>
constexpr math::fixed_t<Q, uint32_t> _IQNimag(math::fixed_t<Q, D> first, Args&&... rest) {
    uint64_t sum = sum_of_squares(first, rest...);
    return math::fixed_t<Q, uint32_t>::from_bits(
        IqSqrtCoeffs::template from_sqsum<Q, SqrtNormStrategy::IMAG>(sum).template compute<Q, SqrtNormStrategy::IMAG>()
    );
}


}
