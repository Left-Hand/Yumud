#include "exprimental.hpp"

#include "core/arch/riscv/riscv_common.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/bus/uart/hw_singleton.hpp"
#include "core/stream/BufStream.hpp"
#include "core/utils/Result.hpp"
#include <cmath>

using namespace ymd;

template<typename GFn, typename EFn>
__no_inline Microseconds eval_one_func(GFn && g_fn, size_t times, EFn && fn){
    const auto begin_us = clock::micros();
    auto y = std::forward<EFn>(fn)(0);
    auto x = uq32(0);
    const auto step = uq32::from_rcp(times * 4);
    for(size_t i = 0; i < times; ++i){
        
        (y) += std::apply(std::forward<EFn>(fn), std::make_tuple(std::forward<GFn>(g_fn)(x)));
        x+= step;
    }

    const auto end_us = clock::micros();
    const auto elapsed = end_us - begin_us;
    DEBUG_PRINTLN(times, elapsed, y);
    return (end_us - begin_us);
}

template<typename GFn, typename Fn1, typename Fn2>
__no_inline auto compare_func(GFn && g_fn, size_t times, Fn1 && fn1, Fn2 && fn2){
    const auto elapsed1 = eval_one_func(std::forward<GFn>(g_fn), times, std::forward<Fn1>(fn1));
    const auto elapsed2 = eval_one_func(std::forward<GFn>(g_fn), times, std::forward<Fn2>(fn2));

    const auto per_call_us1 = uq16(elapsed1.count()) / times;
    const auto per_call_us2 = uq16(elapsed2.count()) / times;
    DEBUG_PRINTLN(per_call_us1, per_call_us2);
    DEBUG_PRINTLN(per_call_us1 * 144, per_call_us2 * 144);
}


__attribute__((always_inline))

[[nodiscard]] static constexpr uint32_t  div_10000( const uint32_t u32_in ) noexcept{
    // constexpr size_t SHIFTS = 43;
    constexpr size_t SHIFTS = 45;
    constexpr uint32_t MAGIC = (1ull << SHIFTS) / 10000 + 1;
    return static_cast<uint32_t>((static_cast<uint64_t>(MAGIC) * (u32_in)) >> SHIFTS);
}

static_assert(div_10000(uint32_t(0xFFFFFFFF)) == 0xFFFFFFFF / 10000);



void test_div_10000(){
    static constexpr size_t NUM_BLOCKS = 256;
    for(uint32_t i = 0; i < NUM_BLOCKS; ++i){
        const auto block_res = [&] -> Result<void, std::tuple<uint32_t, uint32_t, uint32_t>>{
            //将u32分为block个区间 从base开始
            static constexpr size_t BLOCK_SIZE = ((1ull << 32) / NUM_BLOCKS);
            const uint32_t base = i * BLOCK_SIZE;
            const uint32_t end = base + BLOCK_SIZE;

            for(uint32_t n = base; n < end; ++n){
                const uint32_t expected = static_cast<uint32_t>(n / 10000);
                const uint32_t actual = div_10000(n);
                if(expected != actual){
                    return Err(std::make_tuple(n, expected, actual));
                }
            }
            return Ok();
        }();

        if(!block_res.is_ok()){
            DEBUG_PRINTLN(block_res.unwrap_err());
            return;
        }else{
            DEBUG_PRINTLN("block done:", i, '/', NUM_BLOCKS);
        }
    }
    DEBUG_PRINTLN("done");
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


    auto fn1 = [](const iq16 x) -> auto {
        // const auto [s, c] = sincospu_approx(x);
        // return iq20(math::cospu_approx(x));
        return iq20(math::atan2pu(iq16(1),x));
        // return iq20(math::atan2pu(iq16(x),iq16(1)));
        // return iq20(math::cospu(x));
        // return iq20(math::sinpu_approx(x));
        // return iq20(s) + iq20(c);
        // return math::exp(iq16(math::inv_mag(2 * s, c)));
        // return iq16::from_bits(m__IQNdiv_impl<16, true>(s.to_bits(),c.to_bits()));
        // return iq20(s);
    };

    auto fn2 = [](const iq16 x) -> auto {
            // const auto [s, c] = sincospu_approx(x);
            // return math::atan2pu(s,c);
            // return math::atanpu(s/c);
            // return iq20(m__sinpu_approx(x));
            return iq20(math::sqrt(x));
            // return s / c;
            // return iq31(0);
            // const auto fx = float(x);
            // const auto s = std::sin(fx);
            // const auto c = std::cos(fx);
            // return iq20::from(s) + iq20::from(c);
            // return iq20(s);
    };

    std::array<uint8_t, 128> format_buf;
    auto formatter = BufStream (std::span(format_buf));
    // formatter.set_radix(8);
    // formatter.set_splitter(", ");
    formatter.set_splitter(",");
    formatter.set_eps(4);
    while(true){
        const auto now_secs = clock::seconds_precious();
        // const auto x = 2 * iq16(frac(now_secs * 2)) * iq16(2 * M_PI) -  1000 * iq16(2 * M_PI);
        // const auto x = iq16(2 * M_PI) * iq16(math::frac(now_secs * 2));
        // const auto x = pu_to_uq32((now_secs * 2));
        const auto x = uq32::from_bits((now_secs * 2).to_bits());
        const auto [s,c] = math::sincospu(x);
        // const auto x = 6 * frac(t * 2) - 3;

        
        // const auto y1 = fn1(s, c);
        // const auto y2 = fn2(s, c);
        const auto y3 = math::asin(iq16(s));
        const auto y4 = math::acos(iq16(s));
        const auto y5 = math::atan2(iq16(s), iq16(c));
        const auto elapsed_us = measure_total_elapsed_us([&]{
            formatter.println(
                // uq30(x),
                // std::showpos, 
                // std::showbase,
                // std::hex,
                uq16(x),
                uq32(x),
                // (s).to_bits() - fn2(x).to_bits(), 
                c,
                s, fn2(x),
                // std::numeric_limits<iq31>::min(), 
                // std::numeric_limits<iq31>::max(), 
                // y1, y2,
                y3, y4, 
                iq16::from(float(-y5)),
                // -y5,

                int32_t(-1)
                // x.to_bits() >> 24
                // std::oct
                // x.to_bits() >> 24,
            );
        });

        DEBUG_PRINTLN(formatter.collected_str().trim(), elapsed_us.count(), formatter.collected_str().length());
        formatter.reset();
        // clock::delay(1ms);
    }

    compare_func(
        [](const uq16 x){return iq16(x);},
        4096,
        // 32,
        // +fn1, 
        // +fn2
        fn1, 
        fn2
    );
    PANIC{};
}