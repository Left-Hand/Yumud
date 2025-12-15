
#include "core/math/real.hpp"
#include "core/math/iq/iqmath.hpp"
#include "core/math/intrinsics/batch/conv.hpp"
#include "core/math/iq/fixed_t.hpp"
#include "core/arch/riscv/riscv_common.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/bus/uart/uarthw.hpp"

// static constexpr __fast_inline 
// int32_t __UIQ32getSinCosResult(uint32_t iq31X, uint32_t iq31Sin, uint32_t iq31Cos){
//     uint32_t iq31Res;

//     /* 0.333*x*C(k) */
//     iq31Res = __mpyf_l(0x2aaaaaab, iq31X);
//     iq31Res = __mpyf_l(iq31Cos, iq31Res);

//     /* -S(k) - 0.333*x*C(k) */
//     iq31Res = -(iq31Sin + iq31Res);

//     /* 0.5*x*(-S(k) - 0.333*x*C(k)) */
//     iq31Res = iq31Res >> 1;
//     iq31Res = __mpyf_l(iq31X, iq31Res);

//     /* C(k) + 0.5*x*(-S(k) - 0.333*x*C(k)) */
//     iq31Res = iq31Cos + iq31Res;

//     /* x*(C(k) + 0.5*x*(-S(k) - 0.333*x*C(k))) */
//     iq31Res = __mpyf_l(iq31X, iq31Res);

//     /* sin(Radian) = S(k) + x*(C(k) + 0.5*x*(-S(k) - 0.333*x*C(k))) */
//     iq31Res = iq31Sin + iq31Res;

//     return iq31Res;
// }

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


using namespace ymd;


namespace exprimental{
using namespace iqmath::details;

struct [[nodiscard]] IqSincosIntermediate{
    using Self = IqSincosIntermediate;

    struct SinCosResult{
        fixed_t<31, int32_t> sin;
        fixed_t<31, int32_t> cos;

        friend OutputStream & operator << (OutputStream & os, const SinCosResult & obj){
            return os << obj.sin << os.splitter() << obj.cos;
        }
    };

    int32_t iq31_x;
    int32_t iq31_sin;
    int32_t iq31_cos;
    uint8_t sect; 

    __attribute__((always_inline)) constexpr 
    fixed_t<31, int32_t> exact_sin() const {
        //获取查找表的校准值

        switch(sect){
            case 0: return fixed_t<31, int32_t>::from_bits(  exact(iq31_x, iq31_sin,  iq31_cos));
            case 1: return fixed_t<31, int32_t>::from_bits(  exact(iq31_x, iq31_cos, -iq31_sin));
            case 2: return fixed_t<31, int32_t>::from_bits(  exact(iq31_x, iq31_cos, -iq31_sin));
            case 3: return fixed_t<31, int32_t>::from_bits(  exact(iq31_x, iq31_sin,  iq31_cos));
            case 4: return fixed_t<31, int32_t>::from_bits(  exact(iq31_x, -iq31_sin, -iq31_cos));
            case 5: return fixed_t<31, int32_t>::from_bits(  exact(iq31_x, -iq31_cos,  iq31_sin));
            case 6: return fixed_t<31, int32_t>::from_bits(  exact(iq31_x, -iq31_cos,  iq31_sin));
            case 7: return fixed_t<31, int32_t>::from_bits(  exact(iq31_x, -iq31_sin, -iq31_cos));
        }
        __builtin_unreachable();
    }

    __attribute__((always_inline)) constexpr 
    fixed_t<31, int32_t> exact_cos() const {
        switch(sect){
            case 0: return fixed_t<31, int32_t>::from_bits(  exact(iq31_x, iq31_cos, -iq31_sin));
            case 1: return fixed_t<31, int32_t>::from_bits(  exact(iq31_x, iq31_sin,  iq31_cos));
            case 2: return fixed_t<31, int32_t>::from_bits(  exact(iq31_x, -iq31_sin, -iq31_cos));
            case 3: return fixed_t<31, int32_t>::from_bits(  exact(iq31_x, -iq31_cos,  iq31_sin));
            case 4: return fixed_t<31, int32_t>::from_bits(  exact(iq31_x, -iq31_cos,  iq31_sin));
            case 5: return fixed_t<31, int32_t>::from_bits(  exact(iq31_x, -iq31_sin, -iq31_cos));
            case 6: return fixed_t<31, int32_t>::from_bits(  exact(iq31_x, iq31_sin,  iq31_cos));
            case 7: return fixed_t<31, int32_t>::from_bits(  exact(iq31_x, iq31_cos, -iq31_sin));

        }
        __builtin_unreachable();
    }

    __attribute__((always_inline)) constexpr 
    auto exact_sincos() const {

        return SinCosResult{
            exact_sin(),
            exact_cos()
        };
    }

private:
    static constexpr int32_t 
    exact(int32_t iq31X, int32_t iq31Sin, int32_t iq31Cos){
        int32_t iq31Res;

        /* 0.333*x*C(k) */
        iq31Res = __mpyf_l(0x2aaaaaab, iq31X);
        iq31Res = __mpyf_l(iq31Cos, iq31Res);

        /* -S(k) - 0.333*x*C(k) */
        iq31Res = -(iq31Sin + iq31Res);

        /* 0.5*x*(-S(k) - 0.333*x*C(k)) */
        iq31Res = iq31Res >> 1;
        iq31Res = __mpyf_l(iq31X, iq31Res);

        /* C(k) + 0.5*x*(-S(k) - 0.333*x*C(k)) */
        iq31Res = iq31Cos + iq31Res;

        /* x*(C(k) + 0.5*x*(-S(k) - 0.333*x*C(k))) */
        iq31Res = __mpyf_l(iq31X, iq31Res);

        /* sin(Radian) = S(k) + x*(C(k) + 0.5*x*(-S(k) - 0.333*x*C(k))) */
        iq31Res = iq31Sin + iq31Res;

        return iq31Res;
    }

};

template<const size_t Q>
constexpr fixed_t<Q, uint32_t> mysqrt(const fixed_t<Q, uint64_t> x){
    return fixed_t<Q, uint32_t>::from_bits(
        from_single_input_64<Q, TYPE_SQRT>(x.to_bits()
        // iqmath::details::IqSqrtCoeffs::template from_single_u32<Q, TYPE_SQRT>(x.to_bits()
        ).template compute<Q, TYPE_SQRT>()
    );
}



template<size_t _Q, size_t Q = std::min(_Q, size_t(16))>
constexpr IqSincosIntermediate __IQNgetCosSinPU(int32_t iqn_x){
    if constexpr (_Q > 16) iqn_x = iqn_x >> (_Q - 16);
    constexpr int32_t iqn_tau = (1 << Q) * (M_PI * 2);
    //将x取余到[0, 1)之间

    iqn_x = iqn_x & ((1 << Q) - 1);

    constexpr uint32_t eeq_mask = ((1 << (Q-3)) - 1);
    const uint8_t sect = iqn_x >> (Q - 3);
    //将一个周期拆分为八个区块 每个区块长度pi/4 获取区块索引
    
    const uint32_t uiqn_eeq_x = (iqn_x & eeq_mask) * (iqn_tau / 8) >> (Q - 3);
    //将x继续塌陷 从[0, 2 * pi)变为[0, pi/4) 后期通过诱导公式映射到八个区块的任一区块

    const uint32_t uiq31_eeq_x = uiqn_eeq_x << (31 - Q);
    //提高x的q值到31
    
    constexpr uint32_t uiq31_quatpi = uint32_t(uint64_t(1 << 29) * (M_PI));

    const uint32_t uiq31_flip_x = (sect & 0b1) ? (uiq31_quatpi - uiq31_eeq_x) : uiq31_eeq_x;
    //将x由锯齿波变为三角波

    const int32_t iq31_x = uiq31_flip_x & 0x01ffffff;
    //获取每个扇区的偏移值

    const uint8_t lut_index = (uint16_t)(uiq31_flip_x >> 25) & 0x003f;
    //计算查找表索引

    const int32_t iq31_sin = iqmath::details::_IQ31SinLookup[lut_index];
    const int32_t iq31_cos = iqmath::details::_IQ31CosLookup[lut_index];

    return IqSincosIntermediate{
        iq31_x, 
        iq31_sin,
        iq31_cos,
        sect
    };

}



template<size_t _Q, size_t Q = std::min(_Q, size_t(16))>
constexpr IqSincosIntermediate __IQNgetCosSin(int32_t iqn_x){
    if constexpr (_Q > 16) iqn_x = iqn_x >> (_Q - 16);
    constexpr uint32_t uiqn_inv_tau = (1 << Q) / (M_PI * 2);

    //现在直接缩到原来1/pi 调用pu版本 这样减少了一次取余(复杂度与除法相同) 性能提高20%
    //这个函数后面的不用看了
    return __IQNgetCosSinPU<Q>(
        (uint32_t(iqn_x) * uiqn_inv_tau) >> Q);
}


template<size_t Q, typename D>
requires (sizeof(D) == 4)
__fast_inline constexpr 
fixed_t<31, int32_t> mysinpu(const fixed_t<Q, D> iq_x){
    return __IQNgetCosSinPU<Q>(iq_x.to_bits()).exact_cos();
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
fixed_t<31, int32_t> mycospu(const fixed_t<Q, D> x){
    return __IQNgetCosSinPU<Q>(x.to_bits()).exact_sin();
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
std::array<fixed_t<31, int32_t>, 2> mysincospu(const fixed_t<Q, D> x){
    const auto res = __IQNgetCosSinPU<Q>(x.to_bits()).exact_sincos();
    return {res.sin, res.cos};
}

}



template<typename Fn>
__no_inline auto eval_func(Fn && fn){
    auto y = std::forward<Fn>(fn)(clock::time());

    static constexpr size_t times = 10000;

    const auto begin_ms = clock::micros();
    const auto t = clock::time();
    for(size_t i = 0; i < times; ++i){
        // __nop;
        (y) += (std::forward<Fn>(fn)(t));
        // __nop;
        // __nop;
    }

    const auto end_ms = clock::micros();
    // DEBUG_PRINTLN(static_cast<uint32_t>((end_ms - begin_ms).count()) / times );
    DEBUG_PRINTLN(static_cast<uint32_t>((end_ms - begin_ms).count()), y);
    return (end_ms - begin_ms);
}

template<typename Fn>
void test_func(Fn && fn){
    while(true){
        const auto now_secs = clock::time();
        // const auto x = 2 * iq16(frac(now_secs * 2)) * iq16(TAU) -  1000 * iq16(TAU);
        const auto x = 2 * iq16(math::frac(now_secs * 2));
        // const auto x = 6 * frac(t * 2) - 3;
        auto y = std::forward<Fn>(fn)(x);
        DEBUG_PRINTLN_IDLE(x, y);
        clock::delay(1ms);
    }
}

void sincos_main(){
    DEBUGGER_INST.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000 
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.no_brackets(EN);


    clock::delay(200ms);

    auto func = [](const iq16 x) -> auto {
        // return std::sin(x);
        // return exprimental::math::sinpu(static_cast<iq31>(x));
        // return exprimental::mycospu(static_cast<iq31>(x));
        // return exprimental::mysqrt(uuq16::from_bits(x.to_bits()) << 16);
        // return sqrt(uuq16::from_bits(x.to_bits()) << 16);
        // return sqrt(uq16::from_bits(x.to_bits()) << 16);
        // const auto [_s, _c] = sincos(x);
        // const auto s = iq16(_s);
        // const auto c = iq16(_c);
        // return sqrt(s * s + c * c);
        // const auto res = (s * s + c * c);
        // const auto res = sqrt(uuq16::from_bits(static_cast<uint64_t>(std::bit_cast<uint32_t>(x.to_bits())) << 16));
        const auto y_bits = x.to_bits();
        // const auto y_bits = __builtin_clz(x.to_bits());
        // const auto y_bits = __builtin_bitreverse32(x.to_bits());
        // const auto res = sqrt(uuq16::from_bits(static_cast<uint64_t>(std::bit_cast<uint32_t>(y_bits) << 16)));
        const auto res = math::inv_mag(
            iq16::from_bits(static_cast<int32_t>(std::bit_cast<uint32_t>(y_bits))),
            iq16::from_bits(static_cast<int32_t>(std::bit_cast<uint32_t>(y_bits)))
        );
        // if(res < 0) PANIC{s,c,res};
        return (res);
        // return (s * s);
        // return (c * c);
        // return sqrt(uq16::from_bits(x.to_bits()) << 16);
        // return ymd::math::sinpu(x);
    };

    // const auto dur = eval_func(func);
    // PANIC{riscv_has_native_hard_f32};
    // PANIC{has_b_clz};
    // PANIC{riscv_has_native_ctz};
    // PANIC{riscv_has_native_ctz};
    test_func(func);
    PANIC{};
}