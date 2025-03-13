
#include "sys/math/real.hpp"
#include "sys/math/fast/conv.hpp"
#include "sys/math/iq/iq_t.hpp"


#include "types/vector3/Vector3.hpp"
#include "types/quat/Quat.hpp"

#include "sys/debug/debug.hpp"
#include "sys/clock/time.hpp"

#include "sys/utils/Reg.hpp"

/*!
 * @brief Used to specify sine operation
 */
#define TYPE_SIN     (0)
/*!
 * @brief Used to specify cosine operation
 */
#define TYPE_COS     (1)
/*!
 * @brief Used to specify result in radians
 */
#define TYPE_RAD     (0)
/*!
 * @brief Used to specify per-unit result
 */
#define TYPE_PU      (1)

// #define CHIP_SU

using namespace __iqdetails;
namespace experiment{

    // template<const int8_t Q, const int8_t type, const int8_t format>
    // constexpr int32_t __IQNsin_cos_mapx_halfpi(int32_t iqNInput){

    // }


    template<const int8_t Q, const int8_t type, const int8_t format>
    __no_inline constexpr int32_t __IQNsin_cos(int32_t iqNInput){
        bool sign = 0;

        uint32_t uiq30Input;
        uint32_t uiq31Input;
        uint32_t uiq32Input;
        uint32_t uiq31Result = 0;

        /* Remove sign from input */
        if (iqNInput < 0) {
            iqNInput = -iqNInput;

            /* Flip sign only for sin */
            if constexpr(type == TYPE_SIN) {
                sign = 1;
            }
        }


        /* Per unit API */
        if constexpr(format == TYPE_PU) {

            //#region old
            /*
            * Scale input to unsigned iq32 to allow for maximum range. This removes
            * the integer component of the per unit input.
            */
            uiq32Input = (uint32_t)iqNInput << (32 - Q);

            /* Reduce the input to the first two quadrants. */
            if (uiq32Input >= 0x80000000) {
                uiq32Input -= 0x80000000;
                sign ^= 1;
            }

            /*
            * Multiply unsigned iq32 input by 2*pi and scale to unsigned iq30:
            *     iq32 * iq30 = iq30 * 2
            */
            uiq30Input = __mpyf_ul(uiq32Input, _iq30_pi);
            //#endregion


            //#region new
            // /*
            // * Scale input to unsigned iq32 to allow for maximum range. This removes
            // * the integer component of the per unit input.
            // */
            // uiq32Input = (uint32_t)iqNInput << (32 - Q);

            // /* Reduce the input to the first two quadrants. */
            // if (uiq32Input >= 0x80000000) {
            //     uiq32Input -= 0x80000000;
            //     sign ^= 1;
            // }

            // /*
            // * Multiply unsigned iq32 input by 2*pi and scale to unsigned iq30:
            // *     iq32 * iq30 = iq30 * 2
            // */
            // uiq30Input = __mpyf_ul(uiq32Input, _iq30_pi);
            //#endregion
        }
        /* Radians API */
        else {
            //#region old
            // /* Calculate the exponent difference from input format to iq29. */
            // int16_t exp = 29 - Q;

            // /* Save input as unsigned iq29 format. */
            // uiq29Input = (uint32_t)iqNInput;

            // /* Reduce the input exponent to zero by scaling by 2*pi. */
            // while (exp) {
            //     if (uiq29Input >= _iq29_pi) {
            //         uiq29Input -= _iq29_pi;
            //     }
            //     uiq29Input <<= 1;
            //     exp--;
            // }

            // /* Reduce the range to the first two quadrants. */
            // if (uiq29Input >= _iq29_pi) {
            //     uiq29Input -= _iq29_pi;
            //     sign ^= 1;
            // }

            // /* Scale the unsigned iq29 input to unsigned iq30. */
            // uiq30Input = uiq29Input << 1;
            //#endregion

            //#region new
            // /* Calculate the exponent difference from input format to iq29. */
            // int16_t exp = 29 - Q;

            // /* Save input as unsigned iq29 format. */
            // uiq29Input = (uint32_t)iqNInput;

            // /* Reduce the input exponent to zero by scaling by 2*pi. */
            // while (exp) {
            //     if (uiq29Input >= _iq29_pi) {
            //         uiq29Input -= _iq29_pi;
            //     }
            //     uiq29Input <<= 1;
            //     exp--;
            // }

            // /* Reduce the range to the first two quadrants. */
            // if (uiq29Input >= _iq29_pi) {
            //     uiq29Input -= _iq29_pi;
            //     sign ^= 1;
            // }

            // /* Scale the unsigned iq29 input to unsigned iq30. */
            // uiq30Input = uiq29Input << 1;
            //#endregion

            //#region exp
            // /* Calculate the exponent difference from input format to iq29. */
            // int16_t exp = 29 - Q;

            // /* Save input as unsigned iq29 format. */
            // uiq29Input = (uint32_t)iqNInput;

            // /* Reduce the input exponent to zero by scaling by 2*pi. */
            // while (exp) {
            //     if (uiq29Input >= _iq29_pi) {
            //         uiq29Input -= _iq29_pi;
            //     }
            //     uiq29Input <<= 1;
            //     exp--;
            // }

            constexpr auto ds = uint32_t((1 << 29) * (PI/2));
            auto uiq29Input = ((uint32_t)iqNInput << (29 - Q)) % ds;

            /* Reduce the range to the first two quadrants. */
            // if (uiq29Input >= _iq29_pi) {
            //     uiq29Input -= _iq29_pi;
            //     sign ^= 1;
            // }
            
            if (uiq29Input <= 0) {
                uiq29Input += _iq29_pi;
                sign ^= 1;
            }

            /* Scale the unsigned iq29 input to unsigned iq30. */
            uiq30Input = uiq29Input << 1;
            //#endregion
        }

        // /* Reduce the iq30 input range to the first quadrant. */
        // if (uiq30Input >= _iq30_halfPi) {
        //     uiq30Input = _iq30_pi - uiq30Input;

        //     /* flip sign for cos calculations */
        //     if constexpr(type == TYPE_COS) {
        //         sign ^= 1;
        //     }
        // }

        /* Convert the unsigned iq30 input to unsigned iq31 */
        uiq31Input = uiq30Input << 1;

        /* Only one of these cases will be compiled per function. */
        if constexpr(type == TYPE_COS) {
            /* If input is greater than pi/4 use sin for calculations */
            if (uiq31Input > _iq31_quarterPi) {
                uiq31Result = __IQ31calcSin(_iq31_halfPi - uiq31Input);
            } else {
                uiq31Result = __IQ31calcCos(uiq31Input);
            }
        } else if constexpr(type == TYPE_SIN) {
            /* If input is greater than pi/4 use cos for calculations */
            if (uiq31Input > _iq31_quarterPi) {
                uiq31Result = __IQ31calcCos(_iq31_halfPi - uiq31Input);
            } else {
                uiq31Result = __IQ31calcSin(uiq31Input);
            }
        }

        /* set sign */
        if (sign) {
            return -uiq31Result;
        }else{
            return uiq31Result;
        }
    }
    
    // @param iqNInput        IQN type input.
    // @return                IQ31 type result of sin or cosine operation.
    template<int8_t Q>
    constexpr _iq<31> _IQNsin(const _iq<Q> iqNInput){
        return std::bit_cast<_iq<31>>(__IQNsin_cos<Q, TYPE_SIN, TYPE_RAD>(iqNInput.to_i32()));
    }
    
    // @param iqNInput        IQN type input.
    // @return                IQ31 type result of sin or cosine operation.
    template<int8_t Q>
    constexpr _iq<31> _IQNcos(const _iq<Q> iqNInput){
        return std::bit_cast<_iq<31>>(__IQNsin_cos<Q, TYPE_COS, TYPE_RAD>(iqNInput.to_i32()));
    }
    
    // @param iqNInput        IQN type input.
    // @return                IQ31 type result of sin or cosine operation.
    template<int8_t Q>
    constexpr _iq<31> _IQNsinPU(const _iq<Q> iqNInput){
        return std::bit_cast<_iq<31>>(__IQNsin_cos<Q, TYPE_SIN, TYPE_PU>(iqNInput.to_i32()));
    }
    
    
    // @param iqNInput        IQN type input.
    // @return                IQ31 type result of sin or cosine operation.
    template<int8_t Q>
    constexpr _iq<31> _IQNcosPU(const _iq<Q> iqNInput){
        return std::bit_cast<_iq<31>>(__IQNsin_cos<Q, TYPE_COS, TYPE_PU>(iqNInput.to_i32()));
    }
}

using namespace ymd;
using namespace ymd::hal;


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

static constexpr __fast_inline 
int32_t __UIQ32getSinCosResult(uint32_t iq31X, uint32_t iq31Sin, uint32_t iq31Cos){
    uint32_t iq31Res;

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


template<size_t Q>
requires (Q <= 16)
constexpr int32_t _dump_tau(int32_t iqn_x){
    constexpr int32_t iqn_tau = (1 << Q) * (TAU);
    constexpr uint32_t uiqn_inv_tau = (1 << Q) / (TAU);

    iqn_x = iqn_x % iqn_tau;
    if(iqn_x < 0) iqn_x += iqn_tau;
    //将x取余到[0, 2 * pi)之间

    const uint32_t uiqn_norm_x = (uint32_t(iqn_x) * uiqn_inv_tau >> Q);
    //计算x / tau的值 即为[0, 2pi) 之间到[0, 1)之前的锯齿波 以方便提取区块索引

    constexpr uint32_t eeq_mask = ((1 << (Q-3)) - 1);
    const uint8_t sect = uiqn_norm_x >> (Q - 3);
    //将一个周期拆分为八个区块 每个区块长度pi/4 获取区块索引
    
    const uint32_t uiqn_eeq_x = (uiqn_norm_x & eeq_mask) * (iqn_tau / 8) >> (Q - 3);
    //将x继续塌陷 从[0, 2 * pi)变为[0, pi/4) 后期通过诱导公式映射到八个区块的任一区块

    const uint32_t uiq31_eeq_x = uiqn_eeq_x << (31 - Q);
    //提高x的q值到31
    
    constexpr uint32_t uiq31_quatpi = uint32_t(uint64_t(1 << 29) * (PI));

    const uint32_t uiq31_flip_x = (sect & 0b1) ? (uiq31_quatpi - uiq31_eeq_x) : uiq31_eeq_x;
    //将x由锯齿波变为三角波

    const uint8_t lut_index = (uint16_t)(uiq31_flip_x >> 25) & 0x003f;
    //计算查找表索引

    const int32_t iq31_sin = _IQ31SinLookup[lut_index];
    const int32_t iq31_cos = _IQ31CosLookup[lut_index];
    //获取查找表的校准值

    const int32_t iq31_x = uiq31_flip_x & 0x01ffffff;
    //获取每个扇区的偏移值


    switch((sect + 2) & 0b111){
    // switch(sect){
        case 0: return __IQ31getSinCosResult(iq31_x, iq31_sin, iq31_cos);
        case 1: return __IQ31getSinCosResult(iq31_x, iq31_cos, -iq31_sin);
        case 2: return __IQ31getSinCosResult(iq31_x, iq31_cos, -iq31_sin);
        case 3: return __IQ31getSinCosResult(iq31_x, iq31_sin, iq31_cos);
        case 4: return - __IQ31getSinCosResult(iq31_x, iq31_sin, iq31_cos);
        case 5: return - __IQ31getSinCosResult(iq31_x, iq31_cos, -iq31_sin);
        case 6: return - __IQ31getSinCosResult(iq31_x, iq31_cos, -iq31_sin);
        case 7: return - __IQ31getSinCosResult(iq31_x, iq31_sin, iq31_cos);
    }

    // return 0;


    // switch((sect + 2) & 0b111){
    //     case 0: return __iqdetails::__IQ31calcSin(uiq31_quat_x);
    //     case 1: return __iqdetails::__IQ31calcCos(uiq31_quatpi - uiq31_quat_x);
    //     case 2: return __iqdetails::__IQ31calcCos(uiq31_quat_x);
    //     case 3: return __iqdetails::__IQ31calcSin(uiq31_quatpi - uiq31_quat_x);
    //     case 4: return - __iqdetails::__IQ31calcSin(uiq31_quat_x);
    //     case 5: return - __iqdetails::__IQ31calcCos(uiq31_quatpi - uiq31_quat_x);
    //     case 6: return - __iqdetails::__IQ31calcCos(uiq31_quat_x);
    //     case 7: return - __iqdetails::__IQ31calcSin(uiq31_quatpi - uiq31_quat_x);
    // }
    __builtin_unreachable();
}


template<size_t Q>
constexpr iq_t<Q> dump_tau(const iq_t<Q> iqNInput){
    return iq_t<Q>(_iq<Q>::from_i32(_dump_tau<Q>(iqNInput.value.to_i32()) >> (31 - Q)));
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
        const auto x = 2 * frac(t * 2) * real_t(TAU) -  real_t(TAU);
        auto y = std::forward<Fn>(fn)(x);
        DEBUG_PRINTLN(x, y, dur);
        delay(1);
    }
}

#define UART uart2


template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr iq_t<Q> mysin(const iq_t<P> iq){
    return iq_t<Q>(experiment::_IQNsin<P>(iq.value));
}

__no_inline real_t func(const real_t x){
    // return sin(x);
    // return mysin(x);
    // return fposmodp(x, real_t(TAU));
    return dump_tau(iq_t<14>(x));
}
void sincos_main(){
    UART.init(576_KHz);
    DEBUGGER.retarget(&UART);
    DEBUGGER.noBrackets();


    delay(200);

    const uint32_t dur = eval_func(func);
    test_func(func, dur);
}