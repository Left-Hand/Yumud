#pragma once

#include "port.hpp"
#include "constants.hpp"

namespace ymd::fxmath::details{
enum class [[nodiscard]] SqrtNormStrategy {
    SQRT,   // from_u64_nonzero with SqrtNormStrategy::SQRT
    ISQRT,  // from_u64_nonzero with SqrtNormStrategy::ISQRT
    MAG,    // from_sqsum64u_nonzero with SqrtNormStrategy::MAG
    IMAG    // from_sqsum64u_nonzero with SqrtNormStrategy::IMAG
};



struct alignas(4) [[nodiscard]] IqSqrtIntermediate final{
    // ≥ 0x40000000
    uint32_t uiq32_input;

    int32_t i32_exponent;

    template<size_t Q, const SqrtNormStrategy STRATEGY>
    __attribute__((optimize( "-Ofast" )))
    static constexpr IqSqrtIntermediate from_u32_nonzero(uint32_t iqn_input_x) {
        if(iqn_input_x == 0) __builtin_unreachable();
        int32_t i32_exponent;

        /* If the Q gives an odd starting exponent make it even. */
        if constexpr((32 - Q) % 2 == 1) {
            iqn_input_x <<= 1;
            /* Start with positive exponent for sqrt */
            if constexpr(STRATEGY == SqrtNormStrategy::SQRT) {
                i32_exponent = ((32 - Q) - 1) >> 1;
            }
            /* start with negative exponent for isqrt */
            else {
                i32_exponent = -(((32 - Q) - 1) >> 1);
            }
        } else {
            /* start with positive exponent for sqrt */
            if constexpr(STRATEGY == SqrtNormStrategy::SQRT) {
                i32_exponent = (32 - Q) >> 1;
            }
            /* start with negative exponent for isqrt */
            else {
                i32_exponent = -((32 - int32_t(Q)) >> 1);
            }
        }

        /* Save input as unsigned iq32. */
        uint32_t uiq32_input;

        if (iqn_input_x < 0x40000000) [[likely]] {
            int32_t leading_zeros = __builtin_clz(iqn_input_x);
            int32_t shifts_needed = leading_zeros / 2;
            
            uiq32_input = iqn_input_x << (2 * shifts_needed);
            if constexpr(STRATEGY != SqrtNormStrategy::ISQRT) {
                i32_exponent -= shifts_needed;
            } else {
                i32_exponent += shifts_needed;
            }
        }else{
            uiq32_input = iqn_input_x;
        }

        return { 
            .uiq32_input = uiq32_input, 
            .i32_exponent = i32_exponent 
        };
    }

    template<size_t Q, const SqrtNormStrategy STRATEGY>
    __attribute__((optimize( "-Ofast" )))
    static constexpr IqSqrtIntermediate from_u64_nonzero(uint64_t uiiq_n_input_x) {
        if(uiiq_n_input_x == 0) __builtin_unreachable();

        int32_t i32_exponent;

        if constexpr ((32 - Q) % 2 == 1) {
            uiiq_n_input_x <<= 1;
            if constexpr (STRATEGY == SqrtNormStrategy::SQRT) {
                i32_exponent = ((32 - Q) - 1) >> 1;
            } else {
                i32_exponent = -(((32 - Q) - 1) >> 1);
            }
        } else {
            if constexpr (STRATEGY == SqrtNormStrategy::SQRT) {
                i32_exponent = (32 - Q) >> 1;
            } else {
                i32_exponent = -((32 - int32_t(Q)) >> 1);
            }
        }

        uint32_t high = static_cast<uint32_t>(uiiq_n_input_x >> 32);
        uint32_t low  = static_cast<uint32_t>(uiiq_n_input_x);

        int32_t leading_bit_pos;   // 当前 64 位值中最高 1 的位置 (0 … 63)
        if (high != 0) [[likely]] {
            leading_bit_pos = 63 - __builtin_clz(high);
        } else {
            leading_bit_pos = 31 - __builtin_clz(low);   // low 一定非零（已排除 0）
        }

        // ---------- 3. 目标最高位位置：与 leading_bit_pos 同奇偶，30 或 31 ----------
        int32_t target = 30 + (leading_bit_pos & 1);

        // ---------- 4. 计算偶数移位量，一次性完成规范化 ----------
        int32_t shift = leading_bit_pos - target;   // 正数：右移；负数：左移
        int32_t k;                         // 移 2 位的次数
        if (shift >= 0) {
            uiiq_n_input_x >>= shift;
            k = shift >> 1;
            if constexpr (STRATEGY == SqrtNormStrategy::SQRT || 
                        STRATEGY == SqrtNormStrategy::MAG) {
                i32_exponent += k;      // 右移 → 数值变小 → 指数增大
            } else {                  // ISQRT
                i32_exponent -= k;      // 右移 → 逆幅度数值变大 → 指数减小
            }
        } else {
            uiiq_n_input_x <<= -shift;
            k = (-shift) >> 1;
            if constexpr (STRATEGY == SqrtNormStrategy::SQRT || 
                        STRATEGY == SqrtNormStrategy::MAG) {
                i32_exponent -= k;      // 左移 → 数值变大 → 指数减小
            } else {                  // ISQRT
                i32_exponent += k;      // 左移 → 逆幅度数值变小 → 指数增大
            }
        }

        // ---------- 5. 低 32 位即为规范化后的 iq32 数值 ----------
        uint32_t uiq32_input = static_cast<uint32_t>(uiiq_n_input_x);
        // 此时 uiq32_input 一定 ≥ 0x40000000（可加断言）
        if(uiq32_input < 0x40000000) __builtin_unreachable();

        return { 
            .uiq32_input = uiq32_input, 
            .i32_exponent = i32_exponent 
        };
    }

    template<size_t Q, const SqrtNormStrategy STRATEGY>
    __attribute__((optimize( "-Ofast" )))
    static constexpr IqSqrtIntermediate from_sqsum64u_nonzero(uint64_t ui64_sum) {

        if (ui64_sum == 0) __builtin_unreachable();

        int32_t i32_exponent;
        if constexpr (STRATEGY == SqrtNormStrategy::MAG) {
            i32_exponent = (32 - int32_t(Q));      // 幅度：指数初始为正
        } else {
            i32_exponent = (int32_t(Q) - 32);      // 逆幅度：指数初始为负
        }

        /* ------------------------------------------------------------
        *  将64位平方和规范化，使高32位 ≥ 0x40000000。
        *  每次左移2位，同时调整指数。
        *  利用 __builtin_clz 一次计算所需移位次数，消除循环。
        * ------------------------------------------------------------
        */
        uint32_t high = static_cast<uint32_t>(ui64_sum >> 32);
        uint32_t low  = static_cast<uint32_t>(ui64_sum);

        int32_t leading_bit_pos;                          // 整个64位数中最高1的位置 (0 … 63)
        if (high != 0) [[likely]] {
            // 高32位非零：最高位位于高32位内
            leading_bit_pos = 63 - __builtin_clz(high);
        } else {
            // 高32位为零，但整个数非零：最高位位于低32位内
            leading_bit_pos = 31 - __builtin_clz(low);
        }

        #if 0
        // 目标：最高位移至第62位（高32位 ≥ 0x40000000 对应64位的第62位）
        // 需要左移的位数 = 62 - bit_pos，每次移2位 → 次数 k = ceil((62 - leading_bit_pos)/2)
        int32_t k = (62 - leading_bit_pos + 1) / 2;       // 向上取整，且保证非负
        if (k > 0) {
            ui64_sum <<= (2 * k);             // 整体左移
            if constexpr (STRATEGY == SqrtNormStrategy::MAG) {
                i32_exponent -= k;            // 左移使数值变大，幅度指数减小
            } else {
                i32_exponent += k;            // 逆幅度指数增大
            }
        }

        /* 此时高32位一定 ≥ 0x40000000，截取高32位作为iq32格式数值 */
        return {
            static_cast<uint32_t>(ui64_sum >> 32),
            i32_exponent
        };

        #else
        // 目标：最高位移至第62位（高32位 ≥ 0x40000000 对应64位的第62位）
        // 需要左移的位数 = 62 - bit_pos，每次移2位 → 次数 k = ceil((62 - leading_bit_pos)/2)
        uint32_t k = (63 - leading_bit_pos) / 2;       // 向上取整，且保证非负
        ui64_sum <<= (2 * k);             // 整体左移
        if constexpr (STRATEGY == SqrtNormStrategy::MAG) {
            i32_exponent -= k;            // 左移使数值变大，幅度指数减小
        } else {
            i32_exponent += k;            // 逆幅度指数增大
        }

        /* 此时高32位一定 ≥ 0x40000000，截取高32位作为iq32格式数值 */
        return {
            .uiq32_input = static_cast<uint32_t>(ui64_sum >> 32),
            .i32_exponent = i32_exponent
        };
        #endif

    }

    template<size_t Q, const SqrtNormStrategy STRATEGY>
    __attribute__((optimize( "-Ofast" )))
    [[nodiscard]] constexpr uint32_t compute(this auto self) {
        const uint32_t uiq32_input = self.uiq32_input;
        int32_t i32_exponent = self.i32_exponent;

        if(uiq32_input < (32u << 25)) __builtin_unreachable();

        /* Use left most byte as index into lookup table (range: 32-128) */
        uint32_t uiq30_guess = static_cast<uint32_t>(IQ14SQRT_LOOKUP[
            uint32_t(((uiq32_input >> 25) - 32))
        ]) << 16;

        /*
        * Set the loop counter:
        *
        *     iq1 <= Q < 24 - 2 loops
        *     iq22 <= Q <= 31 - 3 loops
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
        {
            #define NEWTON_ITER\
            {\
                if(uiq30_guess & 0x80000000) __builtin_unreachable();\
                const uint32_t uiq31_guess = uiq30_guess << 1;\
                uint32_t uiq32_temp = intrinsics::mul32hu(uiq32_input, uiq31_guess);\
                uint32_t uiq31_temp = (0xC0000000 - intrinsics::mul32hu(uiq32_temp, uiq31_guess));\
                uiq30_guess = intrinsics::mul32hu(uiq31_guess, uiq31_temp);\
            }\

            /* Iterate through Newton-Raphson algorithm. */
            NEWTON_ITER;
            NEWTON_ITER;
            
            if constexpr (Q >= 24) {
                NEWTON_ITER;
            }

            #undef NEWTON_ITER
        }

        uint32_t uiq31_result;
        /* Calculate sqrt(x) for both sqrt and mag */
        if constexpr(STRATEGY == SqrtNormStrategy::SQRT || STRATEGY == SqrtNormStrategy::MAG) {
            /*
            * uiq30_guess contains the inverse square root approximation, multiply
            * by uiq32_input to get square root result.
            */
            if(uiq30_guess & 0x80000000) __builtin_unreachable();
            uiq31_result = intrinsics::mul32hu((uiq30_guess << 1), (uiq32_input));


            /*
            * Shift the result right by 31 - Q.
            */
            i32_exponent -= (31 - Q);

            /* Saturate value for any shift larger than 1 (only need this for mag) */
            if constexpr(STRATEGY == SqrtNormStrategy::MAG) {
                if (i32_exponent > 0) [[unlikely]] {
                    return 0xffffffff;
                }
            }

            /* Shift left by 1 check only needed for iq30 and iq31 mag/sqrt */
            if constexpr(Q >= 30) {
                if (i32_exponent > 0) {
                    uiq31_result <<= 1;
                    return uiq31_result;
                }
            }
        }
        /* Separate handling for isqrt and imag. */
        else {

            /*
            * Shift the result right by 31 - Q, add one since we use the uiq30
            * result without shifting.
            */
            i32_exponent = i32_exponent - (31 - Q) + 1;
            uiq31_result = uiq30_guess;

            /* Saturate any positive non-zero exponent for isqrt. */
            if (i32_exponent > 0) [[unlikely]]{
                return 0xffffffff;
            }
        }


        /* Shift uiq31_result right by -exponent —— 使用 clz 优化 */
        if (i32_exponent >= 0) {
            return uiq31_result;
        }
        
        int32_t shift = -i32_exponent;

        /* 若结果为 0 则直接返回（防御性编程） */
        if (uiq31_result == 0) [[unlikely]] {
            return 0;
        }

        const uint32_t half_rounder = 1u << (shift - 1);
        return (uiq31_result + half_rounder) >> shift;
    }
};


template<size_t Q>
__attribute__((const, optimize( "-Ofast" )))
constexpr math::fixed<Q, uint32_t> sqrt32u_nonzero(const math::fixed<Q, uint32_t> x){
    return math::fixed<Q, uint32_t>::from_bits(
        IqSqrtIntermediate::template from_u32_nonzero<Q, SqrtNormStrategy::SQRT>(
            x.to_bits()
        ).template compute<Q, SqrtNormStrategy::SQRT>()
    );
}

template<size_t Q>
__attribute__((const, optimize( "-Ofast" )))
constexpr math::fixed<Q, uint32_t> inv_sqrt32u_nonzero(const math::fixed<Q, uint32_t> x){
    return math::fixed<Q, uint32_t>::from_bits(
        IqSqrtIntermediate::template from_u32_nonzero<Q, SqrtNormStrategy::ISQRT>(
            x.to_bits()
        ).template compute<Q, SqrtNormStrategy::ISQRT>()
    );
}

template<size_t Q>
__attribute__((const, optimize( "-Ofast" )))
constexpr math::fixed<Q, uint32_t> sqrt64u_nonzero(const math::fixed<Q, uint64_t> x){
    return math::fixed<Q, uint32_t>::from_bits(
        IqSqrtIntermediate::template from_u64_nonzero<Q, SqrtNormStrategy::SQRT>(
            x.to_bits()
        ).template compute<Q, SqrtNormStrategy::SQRT>()
    );
}

template<size_t Q>
__attribute__((const, optimize( "-Ofast" )))
constexpr math::fixed<Q, uint32_t> inv_sqrt64u_nonzero(const math::fixed<Q, uint64_t> x){
    return math::fixed<Q, uint32_t>::from_bits(
        IqSqrtIntermediate::template from_u64_nonzero<Q, SqrtNormStrategy::ISQRT>(
            x.to_bits()
        ).template compute<Q, SqrtNormStrategy::ISQRT>()
    );
}

// 计算单个值的平方（辅助函数）
template<typename T>
__attribute__((always_inline, const, optimize( "-Ofast" )))
constexpr uint64_t __square_value(const T val) {
    using extended_t = std::conditional_t<std::is_signed_v<T>, int64_t, uint64_t>;
    auto bits = static_cast<extended_t>(val.to_bits());
    return static_cast<uint64_t>(bits * bits);
}

// 使用折叠表达式计算多个值的平方和
template<typename... Args>
requires (sizeof...(Args) > 0)
__attribute__((always_inline, const, optimize( "-Ofast" )))
constexpr uint64_t __sum_of_squares(Args ... args) {

    return (__square_value(args) + ...);

}

template<size_t Q>
__attribute__((const, optimize( "-Ofast" )))
constexpr math::fixed<Q, uint32_t> mag_sqsum64u_nonzero(uint64_t sum) {
    return math::fixed<Q, uint32_t>::from_bits(
        IqSqrtIntermediate::template from_sqsum64u_nonzero<Q, SqrtNormStrategy::MAG>(sum)
        .template compute<Q, SqrtNormStrategy::MAG>()
    );
}

template<size_t Q>
__attribute__((const, optimize( "-Ofast" )))
constexpr math::fixed<Q, uint32_t> inv_mag_sqsum64u_nonzero(uint64_t sum) {
    return math::fixed<Q, uint32_t>::from_bits(
        IqSqrtIntermediate::template from_sqsum64u_nonzero<Q, SqrtNormStrategy::IMAG>(sum)
        .template compute<Q, SqrtNormStrategy::IMAG>()
    );
}


}


namespace ymd::math{

template<size_t Q>
constexpr __attribute__((always_inline, const, optimize( "-Ofast" )))
fixed<Q, int32_t> sqrt(const fixed<Q, int32_t> x){
    if(x.to_bits() == 0) return 0;
    if(x.to_bits() < 0) __builtin_trap();
    return fixed<Q, int32_t>(fxmath::details::sqrt32u_nonzero(
        fixed<Q, uint32_t>::from_bits(std::bit_cast<uint32_t>(x.to_bits()))
    ));
}

template<size_t Q>
constexpr __attribute__((always_inline, const, optimize( "-Ofast" )))
fixed<Q, int32_t> ssqrt(const fixed<Q, int32_t> x){
    if(x.to_bits() == 0) return 0;
    if(x.to_bits() < 0){
        return -fixed<Q, int32_t>(fxmath::details::sqrt32u_nonzero(
            fixed<Q, uint32_t>::from_bits(std::bit_cast<uint32_t>(-x.to_bits()))
        ));
    }else{
        return fixed<Q, int32_t>(fxmath::details::sqrt32u_nonzero(
            fixed<Q, uint32_t>::from_bits(std::bit_cast<uint32_t>(x.to_bits()))
        ));
    }
}

template<size_t Q>
constexpr __attribute__((always_inline, const, optimize( "-Ofast" )))
fixed<Q, uint32_t> sqrt(const fixed<Q, uint32_t> x){
    if(x.to_bits() == 0) return 0;
    return fixed<Q, uint32_t>(fxmath::details::sqrt32u_nonzero(x));
}


template<size_t Q>
constexpr __attribute__((always_inline, const, optimize( "-Ofast" )))
fixed<Q, int32_t> sqrt(const fixed<Q, int64_t> x){
    if(x.to_bits() == 0) return 0;
    if(x.to_bits() < 0) __builtin_trap();
    return fixed<Q, int32_t>(fxmath::details::sqrt64u_nonzero(
        fixed<Q, uint64_t>::from_bits(std::bit_cast<uint64_t>(x.to_bits()))
    ));
}

template<size_t Q>
constexpr __attribute__((always_inline, const, optimize( "-Ofast" )))
fixed<Q, int32_t> ssqrt(const fixed<Q, int64_t> x){
    if(x.to_bits() == 0) return 0;
    if(x.to_bits() < 0){
        return -fixed<Q, int32_t>(fxmath::details::sqrt64u_nonzero(
            fixed<Q, uint64_t>::from_bits(std::bit_cast<uint64_t>(-x.to_bits()))
        ));
    }else{
        return fixed<Q, int32_t>(fxmath::details::sqrt64u_nonzero(
            fixed<Q, uint64_t>::from_bits(std::bit_cast<uint64_t>(x.to_bits()))
        ));
    }
}

template<size_t Q>
constexpr __attribute__((always_inline, const, optimize( "-Ofast" )))
fixed<Q, uint32_t> sqrt(const fixed<Q, uint64_t> x){
    if(x.to_bits() == 0) return 0;
    return fixed<Q, uint32_t>(fxmath::details::sqrt64u_nonzero(x));
}

template<size_t Q>
constexpr __attribute__((always_inline, const, optimize( "-Ofast" )))
fixed<Q, int32_t> inv_sqrt(const fixed<Q, int32_t> x){
    if(x.to_bits() == 0) return 0;
    return fixed<Q, int32_t>(fxmath::details::inv_sqrt32u_nonzero(
        fixed<Q, uint32_t>::from_bits(std::bit_cast<uint32_t>(x.to_bits()))
    ));
}

template<size_t Q>
constexpr __attribute__((always_inline, const, optimize( "-Ofast" )))
fixed<Q, uint32_t> inv_sqrt(const fixed<Q, uint32_t> x){
    if(x.to_bits() == 0) return 0;
    return fixed<Q, uint32_t>(fxmath::details::inv_sqrt32u_nonzero<Q>(x));
}


template<typename D, size_t Q, typename... Args>
constexpr __attribute__((always_inline, const, optimize( "-Ofast" )))
fixed<Q, uint32_t> mag(const fixed<Q, D> first, Args ... rest) {
    uint64_t sum = fxmath::details::__sum_of_squares(first, rest...);
    if(sum == 0) return 0;
    return fxmath::details::mag_sqsum64u_nonzero<Q>(sum);
}

template<typename D, size_t Q, typename... Args>
constexpr __attribute__((always_inline, const, optimize( "-Ofast" )))
fixed<Q, uint32_t> inv_mag(const fixed<Q, D> first, Args ... rest) {
    uint64_t sum = fxmath::details::__sum_of_squares(first, rest...);
    if(sum == 0) return 0;
    return fxmath::details::inv_mag_sqsum64u_nonzero<Q>(sum);
}

}