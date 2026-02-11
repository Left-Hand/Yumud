#include "exprimental.hpp"

#include "core/arch/riscv/riscv_common.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/bus/uart/hw_singleton.hpp"
#include <cmath>

using namespace ymd;

template<typename Fn>
__no_inline Microseconds eval_one_func(size_t times, Fn && fn){
    const auto begin_us = clock::micros();
    
    auto y = std::forward<Fn>(fn)(0, 0);
    auto x = uq32(0);
    const auto step = uq32::from_rcp(times * 4);
    for(size_t i = 0; i < times; ++i){
        auto [s,c] = math::sincos_approx(x);
        (y) += (std::forward<Fn>(fn)(s,c));
        x+= step;
    }

    const auto end_us = clock::micros();
    const auto elapsed = end_us - begin_us;
    DEBUG_PRINTLN(times, elapsed, y);
    return (end_us - begin_us);
}

template<typename Fn1, typename Fn2>
__no_inline auto compare_func(size_t times, Fn1 && fn1, Fn2 && fn2){
    const auto elapsed1 = eval_one_func(times, std::forward<Fn1>(fn1));
    const auto elapsed2 = eval_one_func(times, std::forward<Fn2>(fn2));
    DEBUG_PRINTLN(uq16(elapsed1.count()) / times, uq16(elapsed2.count()) / times);
}


template<typename Fn>
__no_inline auto eval_func(Fn && fn){
    auto y = std::forward<Fn>(fn)(clock::seconds());

    static constexpr size_t times = 10000;

    const auto begin_us = clock::micros();
    const auto t = clock::seconds();
    for(size_t i = 0; i < times; ++i){
        // __nop;
        // (y) += (std::forward<Fn>(fn)(t));
        // __nop;
        // __nop;
    }

    const auto end_us = clock::micros();
    // DEBUG_PRINTLN(static_cast<uint32_t>((end_us - begin_us).count()) / times );
    DEBUG_PRINTLN(static_cast<uint32_t>((end_us - begin_us).count()), y);
    return (end_us - begin_us);
}


template<int8_t Q, bool IS_SIGNED>
constexpr int32_t m__IQNdiv_impl(int32_t iqNInput1, int32_t iqNInput2)
{
    using namespace iqmath::details;
    bool is_neg = 0;
    uint32_t uiqNResult;

    if constexpr(IS_SIGNED == true) {
        /* save sign of denominator */
        if (iqNInput2 == 0) [[unlikely]]{
            return INT32_MAX;
        }else if(iqNInput2 < 0){
            if(iqNInput2 == INT32_MIN) [[unlikely]] {
                iqNInput2 = INT32_MAX;
                is_neg = 1;
            }else{
                iqNInput2 = -iqNInput2;
                is_neg = 1;
            }
        }

        /* save sign of numerator */
        if (iqNInput1 < 0) {
            is_neg = !is_neg;

            if(iqNInput1 == INT32_MIN) [[unlikely]] {
                iqNInput1 = INT32_MAX;
            }else{
                iqNInput1 = -iqNInput1;
            }
        }

    } else {
        /* Check for divide by zero */
        if (iqNInput2 == 0) [[unlikely]] {
            return INT32_MAX;
        }
    }


    /* Scale inputs so that 0.5 <= uiq32Input2 < 1.0. */
    // Handle zero case to avoid undefined behavior in __builtin_clz
    // Find the number of leading zeros to determine the shift amount
    #if 0
    //1.046us per call @ch32v303 144mhz(fpu present)
    #if 0
    const size_t shift_amount = [&] -> size_t __no_inline{
        return size_t(CLZ(iqNInput2));
    }();
    #else
    const size_t shift_amount = size_t(CLZ(iqNInput2));
    #endif
    #else
    //0.79us per call @ch32v303 144mhz(fpu present)
    const size_t shift_amount = __builtin_clz(iqNInput2);
    #endif

    if(shift_amount >= 32) __builtin_unreachable();
    
    uint32_t uiq32Input2 = iqNInput2 << shift_amount;
    uint64_t uiiqNInput1 = uint64_t(iqNInput1);
    if constexpr(Q < 31) {
        const int32_t shifts = (31 - Q - 1 - shift_amount);
        if(shifts >= 0) {
            uiiqNInput1 >>= shifts;
        } else {
            uiiqNInput1 <<= -shifts;
        }
    } else {
        uiiqNInput1 <<= 1 + shift_amount;
    }

    /*
     * Shift input1 back from iq31 to iqN but scale by 2 since we multiply
     * by result in iq30 format.
     */


    /* Check for saturation. */
    if (uint32_t(uiiqNInput1 >> 32)) {
        if (is_neg) {
            return INT32_MIN;
        } else {
            return INT32_MAX;
        }
    }

    /* use left most 7 bits as ui8Index into lookup table (range: 32-64) */
    const size_t ui8Index = size_t((uiq32Input2 >> 25) - 64);
    uint32_t uiq30Guess = uint32_t(_IQ6div_lookup[ui8Index]) << 24;

    

    uint32_t ui30Temp;

    /* 牛顿迭代 lambda - 无分支 */
    auto newton_iter = [&]() __attribute__((always_inline)){
        ui30Temp = static_cast<uint32_t>(
            static_cast<uint64_t>(uiq30Guess) * 
            static_cast<uint64_t>(uiq32Input2) >> 32
        );
        ui30Temp = 0x80000000 - ui30Temp;  /* - (temp - 0x80000000) */
        uiq30Guess = static_cast<uint64_t>(uiq30Guess) * 
                    static_cast<uint64_t>(ui30Temp) >> 32;
        uiq30Guess <<= 2;
    };

    newton_iter();
    newton_iter();
    
    if constexpr (Q >= 24) {
        newton_iter();
    }

    /* Multiply 1/uiq32Input2 and uiqNInput1. */
    uiqNResult = (static_cast<uint64_t>(uiq30Guess) * static_cast<uint64_t>(uint32_t(uiiqNInput1))) >> 32;


    /* Saturate, add the sign and return. */
    if constexpr(IS_SIGNED == true) {
        if(is_neg){
            if(uiqNResult > uint32_t(INT32_MIN)) [[unlikely]] {
                return INT32_MIN;
            }
            return -(int32_t)uiqNResult;
        }else{
            if(uiqNResult > uint32_t(INT32_MAX)) [[unlikely]] {
                return INT32_MAX;
            }
            return (int32_t)uiqNResult;
        }
    } else {
        return uiqNResult;
    }
}


template<typename Fn>
void play_func(Fn && fn){
    while(true){
        const auto now_secs = clock::seconds();
        // const auto x = 2 * iq16(frac(now_secs * 2)) * iq16(2 * M_PI) -  1000 * iq16(2 * M_PI);
        // const auto x = iq16(2 * M_PI) * iq16(math::frac(now_secs * 2));
        const auto x = pu_to_uq32((now_secs * 2));
        // const auto x = 6 * frac(t * 2) - 3;
        auto y = std::forward<Fn>(fn)(x);
        DEBUG_PRINTLN_IDLE(
            // x, 
            std::get<0>(y),
            std::get<1>(y)
            // std::sin(2 * M_PI * float(iq20(x))),
            // 100000 * (float(std::get<0>(y)) - std::sin(2 * M_PI * float(iq20(x)))), 
            // std::get<0>(y).to_bits() >> 16, 
            // std::get<1>(y).to_bits() >> 16, 
            // dual_iq31_length_squared(std::get<0>(y), std::get<1>(y)).to_bits() >> 16, 

            // std::get<2>(y).to_bits() >> 16, 
            // std::get<3>(y).to_bits() >> 16, 

            // dual_iq31_length_squared(std::get<2>(y), std::get<3>(y)).to_bits(), 
            // ((std::get<0>(y).to_bits() >> 16) - (std::get<2>(y).to_bits() >> 16)),
            // ((std::get<1>(y).to_bits() >> 16) - (std::get<3>(y).to_bits() >> 16)),

            // dual_iq31_length_squared((std::get<0>(y) - std::get<2>(y)),
            // (std::get<1>(y) - std::get<3>(y)).to_bits()
        );
        clock::delay(1ms);
    }
}

void sincos_main(){
    DEBUGGER_INST.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz), 
        // .baudrate = hal::NearestFreq(6000000), 
        .tx_strategy = CommStrategy::Blocking,
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.no_brackets(EN);
    // DEBUGGER.set_eps(4);
    DEBUGGER.set_eps(6);


    clock::delay(200ms);

    // const auto dur = eval_func(func);
    // PANIC{riscv_has_native_hard_f32};
    // PANIC{has_b_clz};
    // PANIC{riscv_has_native_ctz};
    // PANIC{riscv_has_native_ctz};


    if(false)while(true){
    // while(true){
        static uq32 x = 0;
        constexpr uq32 step = uq32::from_rcp(1024u);
        x += step;
        // const auto x = 2 * iq16(frac(now_secs * 2)) * iq16(2 * M_PI) -  1000 * iq16(2 * M_PI);
        // const auto x = iq16(2 * M_PI) * iq16(math::frac(now_secs * 2));
        // const auto x = 6 * frac(t * 2) - 3;
        const auto [_s, _c] = math::sincospu(x);
        const auto s = _s;
        const auto c = _c;
        // const auto [s, c] = math::sincospu_approx(x);
        DEBUG_PRINTLN_IDLE(
            // x, 
            s, c, 
            math::atan2pu(s, c),
            (int64_t)math::pu_to_uq32(math::atan2pu(s, c)).to_bits() - x.to_bits()
            // (int64_t)math::atan2pu(s, c).to_bits(),
            // x.to_bits()
            // math::atan2pu(s, c),
            // iq16(math::atanpu(s / c)),

            // math::atan2(s, c),
            // iq16(math::atan(s / c))
        );
        // clock::delay(1ms);
    }
    if(false){
        uq32 x = 0;
        iq24 y = 0;
        constexpr uq32 step = uq32::from_rcp(32u);
        for(size_t i = 0; i < 32; ++i){
            y += iq24(std::get<0>(math::sincospu(x)));
            x += step;
        }
        DEBUG_PRINTLN(y);
    }


    auto fn1 = [](const iq16 s, const iq16 c) -> auto {
        // const auto [s, c] = sincospu_approx(x);
        // const auto [s, c] = math::sincospu_approx(x);
        // return iq20(s) + iq20(c);
        return math::inv_mag(2 * s, c);
        // return iq16::from_bits(m__IQNdiv_impl<16, true>(s.to_bits(),c.to_bits()));
        // return iq20(s);
    };

    auto fn2 = [](const iq16 s, const iq16 c) -> auto {
            // const auto [s, c] = sincospu_approx(x);
            // return math::atan2pu(s,c);
            // return math::atanpu(s/c);
            return s / c;
            // return iq31(0);
            // const auto fx = float(x);
            // const auto s = std::sin(fx);
            // const auto c = std::cos(fx);
            // return iq20::from(s) + iq20::from(c);
            // return iq20(s);
    };

    while(true){
        const auto now_secs = clock::seconds();
        // const auto x = 2 * iq16(frac(now_secs * 2)) * iq16(2 * M_PI) -  1000 * iq16(2 * M_PI);
        // const auto x = iq16(2 * M_PI) * iq16(math::frac(now_secs * 2));
        const auto x = pu_to_uq32((now_secs * 2));
        const auto [s,c] = math::sincospu(x);
        // const auto x = 6 * frac(t * 2) - 3;
        DEBUG_PRINTLN(
            x,
            s, c,
            fn1(s, c),
            fn2(s, c),
            math::asin(iq16(s)),
            math::acos(iq16(s)),
            fn1(s,c).to_bits() - fn2(s, c).to_bits()
        );
        clock::delay(1ms);
    }

    compare_func(
        2048,
        // 32,
        fn1, 
        fn2
    );
    PANIC{};
}