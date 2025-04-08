
#include "core/math/real.hpp"
#include "core/math/fast/conv.hpp"
#include "core/math/iq/iq_t.hpp"


#include "types/vector3/Vector3.hpp"
#include "types/quat/Quat.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "core/utils/Reg.hpp"

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



using namespace ymd;
using namespace ymd::hal;

namespace expeprimental{
static constexpr __fast_inline 
int32_t __IQ31getSinCosResult(int32_t iq31X, int32_t iq31Sin, int32_t iq31Cos){
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


template<size_t Q, typename Fn>
// requires (Q <= 16)
constexpr auto __IQNgetCosSinPUTemplate(int32_t iqn_x, Fn && fn){
    constexpr int32_t iqn_tau = (1 << Q) * (TAU);

    iqn_x = iqn_x & ((1 << Q) - 1);
    //将x取余到[0, 1)之间

    constexpr uint32_t eeq_mask = ((1 << (Q-3)) - 1);
    const uint8_t sect = iqn_x >> (Q - 3);
    //将一个周期拆分为八个区块 每个区块长度pi/4 获取区块索引
    
    const uint32_t uiqn_eeq_x = (iqn_x & eeq_mask) * (iqn_tau / 8) >> (Q - 3);
    //将x继续塌陷 从[0, 2 * pi)变为[0, pi/4) 后期通过诱导公式映射到八个区块的任一区块

    const uint32_t uiq31_eeq_x = uiqn_eeq_x << (31 - Q);
    //提高x的q值到31
    
    constexpr uint32_t uiq31_quatpi = uint32_t(uint64_t(1 << 29) * (PI));

    const uint32_t uiq31_flip_x = (sect & 0b1) ? (uiq31_quatpi - uiq31_eeq_x) : uiq31_eeq_x;
    //将x由锯齿波变为三角�?

    const int32_t iq31_x = uiq31_flip_x & 0x01ffffff;
    //获取每个扇区的偏移�?

    const uint8_t lut_index = (uint16_t)(uiq31_flip_x >> 25) & 0x003f;
    //计算查找表索�?


    return std::forward<Fn>(fn)(iq31_x, sect, lut_index);
    // return _iq<31>::from_i32(iq31_sin+iq31_cos);
}



template<size_t Q, typename Fn>
requires (Q <= 16)
constexpr auto __IQNgetCosSinTemplate(int32_t iqn_x, Fn && fn){
    constexpr int32_t iqn_tau = (1 << Q) * (TAU);
    constexpr uint32_t uiqn_inv_tau = (1 << Q) / (TAU);

    return __IQNgetCosSinPUTemplate<Q>(
        (uint32_t(iqn_x) * uiqn_inv_tau) >> Q, std::forward<Fn>(fn));
    //现在直接缩到原来1/pi 调用pu版本 这样减少了一次取�?(复杂度与除法相同) 性能提高20%
    //这个函数后面的不用看�?

    iqn_x = iqn_x % iqn_tau;
    if(iqn_x < 0) iqn_x += iqn_tau;
    //将x取余到[0, 2 * pi)之间

    const uint32_t uiqn_norm_x = (uint32_t(iqn_x) * uiqn_inv_tau >> Q);
    //计算x / tau的�? 即为[0, 2pi) 之间到[0, 1)之前的锯齿波 以方便提取区块索�?

    constexpr uint32_t eeq_mask = ((1 << (Q-3)) - 1);
    const uint8_t sect = uiqn_norm_x >> (Q - 3);
    //将一个周期拆分为八个区块 每个区块长度pi/4 获取区块索引
    
    const uint32_t uiqn_eeq_x = (uiqn_norm_x & eeq_mask) * (iqn_tau / 8) >> (Q - 3);
    //将x继续塌陷 从[0, 2 * pi)变为[0, pi/4) 后期通过诱导公式映射到八个区块的任一区块

    const uint32_t uiq31_eeq_x = uiqn_eeq_x << (31 - Q);
    //提高x的q值到31
    
    constexpr uint32_t uiq31_quatpi = uint32_t(uint64_t(1 << 29) * (PI));

    const uint32_t uiq31_flip_x = (sect & 0b1) ? (uiq31_quatpi - uiq31_eeq_x) : uiq31_eeq_x;
    //将x由锯齿波变为三角�?

    const int32_t iq31_x = uiq31_flip_x & 0x01ffffff;
    //获取每个扇区的偏移�?

    const uint8_t lut_index = (uint16_t)(uiq31_flip_x >> 25) & 0x003f;
    //计算查找表索�?

    return std::forward<Fn>(fn)(iq31_x, sect, lut_index);
}


__fast_inline constexpr 
auto __IQ31getSinDispatcher(const uint32_t iq31_x, const uint8_t sect, const uint8_t lut_index){

    const int32_t iq31_sin = __iqdetails::_IQ31SinLookup[lut_index];
    const int32_t iq31_cos = __iqdetails::_IQ31CosLookup[lut_index];
    //获取查找表的校准�?

    switch(sect){
        case 0: return _iq<31>::from_i32(  __IQ31getSinCosResult(iq31_x, iq31_sin,  iq31_cos));
        case 1: return _iq<31>::from_i32(  __IQ31getSinCosResult(iq31_x, iq31_cos, -iq31_sin));
        case 2: return _iq<31>::from_i32(  __IQ31getSinCosResult(iq31_x, iq31_cos, -iq31_sin));
        case 3: return _iq<31>::from_i32(  __IQ31getSinCosResult(iq31_x, iq31_sin,  iq31_cos));
        case 4: return _iq<31>::from_i32(- __IQ31getSinCosResult(iq31_x, iq31_sin,  iq31_cos));
        case 5: return _iq<31>::from_i32(- __IQ31getSinCosResult(iq31_x, iq31_cos, -iq31_sin));
        case 6: return _iq<31>::from_i32(- __IQ31getSinCosResult(iq31_x, iq31_cos, -iq31_sin));
        case 7: return _iq<31>::from_i32(- __IQ31getSinCosResult(iq31_x, iq31_sin,  iq31_cos));
    }
    __builtin_unreachable();
}

__fast_inline constexpr 
auto __IQ31getCosDispatcher(const uint32_t iq31_x, const uint8_t sect, const uint8_t lut_index){
    return __IQ31getSinDispatcher(iq31_x, (sect + 2) & 0b111, lut_index);
}

__fast_inline constexpr 
auto __IQ31getSinCosDispatcher(const uint32_t iq31_x, const uint8_t sect, const uint8_t lut_index){
    return std::array<_iq<31>, 2>{
        __IQ31getSinDispatcher(iq31_x, sect, lut_index),
        __IQ31getCosDispatcher(iq31_x, sect, lut_index)
    };
}

}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr iq_t<Q> mysin(const iq_t<P> iq_x){
    return iq_t<Q>(expeprimental::__IQNgetCosSinTemplate<Q>(iq_x.value.to_i32(), expeprimental::__IQ31getSinDispatcher));
}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr iq_t<Q> mycos(const iq_t<P> iq_x){
    return iq_t<Q>(expeprimental::__IQNgetCosSinTemplate<Q>(iq_x.value.to_i32(), expeprimental::__IQ31getCosDispatcher));
}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr std::array<iq_t<Q>, 2> mysincos(const iq_t<P> iq_x){
    auto res = (expeprimental::__IQNgetCosSinTemplate<Q>(iq_x.value.to_i32(), expeprimental::__IQ31getSinCosDispatcher));
    return {res[0], res[1]};
}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr iq_t<Q> mysinpu(const iq_t<P> iq_x){
    return iq_t<Q>(expeprimental::__IQNgetCosSinPUTemplate<Q>(iq_x.value.to_i32(), expeprimental::__IQ31getSinDispatcher));
}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr iq_t<Q> mycospu(const iq_t<P> iq_x){
    return iq_t<Q>(expeprimental::__IQNgetCosSinPUTemplate<Q>(iq_x.value.to_i32(), expeprimental::__IQ31getCosDispatcher));
}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr std::array<iq_t<Q>, 2> mysincospu(const iq_t<P> iq_x){
    auto res = (expeprimental::__IQNgetCosSinPUTemplate<Q>(iq_x.value.to_i32(), expeprimental::__IQ31getSinCosDispatcher));
    return {res[0], res[1]};
}



template<typename Fn>
__no_inline uint32_t eval_func(Fn && fn){
    auto y = std::forward<Fn>(fn)(time());

    static constexpr size_t times = 10000;

    const auto begin_m = micros();
    const auto t = time();
    for(size_t i = 0; i < times; ++i){
        __nop;
        (y) = (std::forward<Fn>(fn)(t));
        __nop;
        // __nop;
    }

    const auto end_m = micros();
    // DEBUG_PRINTLN(uint32_t(end_m - begin_m) / times);
    return uint32_t((end_m - begin_m));
}

template<typename Fn>
void test_func(Fn && fn, const uint32_t dur){
    while(true){
        const auto t = time();
        const auto x = 2 * frac(t * 2) * real_t(TAU) -  1000 * real_t(TAU);
        // const auto x = 6 * frac(t * 2) - 3;
        auto y = std::forward<Fn>(fn)(x);
        DEBUG_PRINTLN(x, y, dur);
        delay(1);
    }
}

#define UART hal::uart2



__no_inline auto func(const real_t x){
    // return std::make_tuple(sin(x), cos(x));
    // return mysin(x);
    // return fposmodp(x, real_t(TAU));
    // return dump_tau(iq_t<16>(x));
    // return std::make_tuple(mysin(x), mycos(x));
    // return mysincos(x);
    return mysincospu(x);
    // return mysinpu(x); 
    // return mysinpu(x); 
}
void sincos_main(){
    UART.init(576_KHz);
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets();


    delay(200);

    const uint32_t dur = eval_func(func);
    test_func(func, dur);
}