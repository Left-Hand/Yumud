#include "exprimental.hpp"

#include "core/arch/riscv/riscv_common.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/bus/uart/uarthw.hpp"
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
    DEBUG_PRINTLN(elapsed1, elapsed2);
}


template<typename Fn>
__no_inline auto eval_func(Fn && fn){
    auto y = std::forward<Fn>(fn)(clock::time());

    static constexpr size_t times = 10000;

    const auto begin_us = clock::micros();
    const auto t = clock::time();
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



template<typename Fn>
void play_func(Fn && fn){
    while(true){
        const auto now_secs = clock::time();
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
        // .baudrate = hal::NearestFreq(576_KHz), 
        .baudrate = hal::NearestFreq(6000000), 
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

    compare_func(
        1024,
        // 32,
        [](const iq24 s, const iq24 c) -> auto {
            // const auto [s, c] = sincospu_approx(x);
            // const auto [s, c] = math::sincospu_approx(x);
            // return iq20(s) + iq20(c);
            return math::atan2pu(s,c);
            // return iq20(s);
        },
        [](const iq31 s, const iq31 c) -> auto {
            // const auto [s, c] = sincospu_approx(x);
            return math::atan2pu(s,c);
            // return iq31(0);
            // const auto fx = float(x);
            // const auto s = std::sin(fx);
            // const auto c = std::cos(fx);
            // return iq20::from(s) + iq20::from(c);
            // return iq20(s);
        }
    );
    PANIC{};
}