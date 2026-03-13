#include "exprimental.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/conn/uart/hw_singleton.hpp"
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
        // (y) += (std::forward<EFn>(fn(std::forward<GFn>(g_fn)(x))));
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


// template<typename Fn>
// __no_inline auto eval_func(Fn && fn){
//     auto y = std::forward<Fn>(fn)(clock::seconds());

//     static constexpr size_t times = 10000;

//     const auto begin_us = clock::micros();
//     const auto t = clock::seconds();
//     for(size_t i = 0; i < times; ++i){
//         // __nop;
//         // (y) += (std::forward<Fn>(fn)(t));
//         // __nop;
//         // __nop;
//     }

//     const auto end_us = clock::micros();
//     // DEBUG_PRINTLN(static_cast<uint32_t>((end_us - begin_us).count()) / times );
//     DEBUG_PRINTLN(static_cast<uint32_t>((end_us - begin_us).count()), y);
//     return (end_us - begin_us);
// }



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

// #define MAY_INLINE __no_inline constexpr
#define MAY_INLINE constexpr
// #define MAY_INLINE __attribute__((always_inline)) constexpr
// #define MAY_INLINE 

#if 0
__attribute__((always_inline))
static int32_t taylor_2o_perfect(uint32_t t, int32_t S, int32_t C) {
    #if 0
    int32_t res;
    int32_t tC, tSh, t2S, th;
    
    
    __asm__ (
        "mulhsu  %[tC], %[C], %[t]       \n\t"  // tC = C*t
        "srli    %[th], %[t], 1          \n\t"  // th = t/2
        "mulhsu  %[tSh], %[S], %[th]     \n\t"  // tSh = S*(t/2)
        "add     %[S], %[S], %[tC]       \n\t"  // S = S + C*t
        "mulhsu  %[t2S], %[tSh], %[t]    \n\t"  // t2S = S*(t/2)*t = S*t²/2
        "sub     %[res], %[S], %[t2S]    \n\t"  // res = S + C*t - S*t²/2
        : [res] "=r" (res),
          [S] "+r" (S),
          [tC] "=&r" (tC),
          [tSh] "=&r" (tSh),
          [t2S] "=&r" (t2S),
          [th] "=&r" (th)
        : [t] "r" (t),
          [C] "r" (C)
        : "cc"
    );
    
    return res;
    #else
    return t + S +C;
    #endif
}
#else


__attribute__((always_inline))
static constexpr int32_t taylor_2o_perfect(uint32_t t, int32_t S, int32_t C) {
    // 计算 C*t
    int32_t tC = fxmath::details::mul32hsu(C, t);
    
    // 计算 t/2
    uint32_t th = t >> 1;
    
    // 计算 S*(t/2)
    int32_t tSh = fxmath::details::mul32hsu(S, th);
    
    // S = S + C*t
    S = S + tC;
    
    // 计算 S*t²/2 = [S*(t/2)] * t
    int32_t t2S = fxmath::details::mul32hsu(tSh, t);
    
    // 最终结果: S + C*t - S*t²/2
    int32_t res = S - t2S;
    
    return res;
}


#endif

// 常数：1/3 和 1/6 的Q31表示
static constexpr uint32_t ONE_THIRD_Q31 = 0x15555555;  // (1/3) 的Q31近似
static constexpr uint32_t ONE_SIXTH_Q31 = 0x0AAAAAAB;  // (1/6) 的Q31近似
__attribute__((always_inline))
static constexpr int32_t taylor_3o_optimized(uint32_t t, int32_t S, int32_t C) {
    // 并行计算独立项
    int32_t tC = fxmath::details::mul32hsu(C, t);           // C*t
    uint32_t t_half = t >> 1;               // t/2
    
    // 计算 S*t/2 和 S*t²/6 可以并行
    int32_t S_t_half = fxmath::details::mul32hsu(S, t_half); // S*t/2
    int32_t C_t_half = fxmath::details::mul32hsu(C, t_half); // C*t/2 (用于t³项)
    
    // S + C*t
    int32_t S_plus_tC = S + tC;
    
    // 计算 t²项: -S*t²/2
    int32_t t2_term = fxmath::details::mul32hsu(S_t_half, t);  // S*t²/2
    
    // 计算 t³项: -C*t³/6
    // C*t/2 * t = C*t²/2
    int32_t C_t2_half = fxmath::details::mul32hsu(C_t_half, t);  // C*t²/2
    // 再乘 t/3 得到 C*t³/6
    int32_t t3_term = fxmath::details::mul32hsu(C_t2_half, fxmath::details::mul32hu(t, ONE_THIRD_Q31));  // C*t³/6
    
    // 最终结果: S + C*t - S*t²/2 - C*t³/6
    int32_t result = S_plus_tC - t2_term - t3_term;
    
    return result;
}



template<typename Fn>
__attribute__((always_inline, optimize("-Ofast"))) constexpr 
math::fixed<31, int32_t> ds_exact_sin(fxmath::details::SincosIntermediate inter, Fn&& taylor_law) {

    #if 0
        //获取查找表的校准值
        int32_t a, b;
        switch(inter.sect_num){
            case 0:
                a = inter.iq31_sin_coeff;
                b =  inter.iq31_cos_coeff;
                break;
            case 1:
                a = inter.iq31_cos_coeff;
                b = -inter.iq31_sin_coeff;
                break;
            case 2:
                a = inter.iq31_cos_coeff;
                b = -inter.iq31_sin_coeff;
                break;
            case 3:
                a = inter.iq31_sin_coeff;
                b =  inter.iq31_cos_coeff;
                break;
            case 4:
                a = -inter.iq31_sin_coeff;
                b = -inter.iq31_cos_coeff;
                break;
            case 5:
                a = -inter.iq31_cos_coeff;
                b =  inter.iq31_sin_coeff;
                break;
            case 6:
                a = -inter.iq31_cos_coeff;
                b =  inter.iq31_sin_coeff;
                break;
            case 7:
                a = -inter.iq31_sin_coeff;
                b = -inter.iq31_cos_coeff;
                break;
            default:
                __builtin_unreachable();
        }
        return math::fixed<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(inter.uq32_x_offset, a, b));
    #else
    int32_t a = inter.iq31_sin_coeff;
    int32_t b =  inter.iq31_cos_coeff;

    bool b01 = bool(inter.sect_num & 0b10) ^ bool(inter.sect_num & 0b01);
    if(b01) std::swap(a, b);
    const bool b2 = (inter.sect_num & 0b100);

    #if 0
    if(b2) a = -a;
    if(b01 ^ b2) b = -b;
    #else
    a = uint32_t(-(b2)) ^ a;
    b = uint32_t(-(b01 ^ b2)) ^ b;
    #endif


    return math::fixed<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(inter.uq32_x_offset, a, b));
    #endif
}

__attribute__((always_inline,  optimize( "-Ofast" )))
constexpr std::tuple<uint32_t, uint32_t> left_shift_two_part(const uint32_t x, const size_t shift){
    #if 0
    const uint32_t rhs = 1u << shift;
    const uint32_t low = fxmath::details::mul32(x, rhs);
    const uint32_t high = fxmath::details::mul32hu(x, rhs);
    #else
    const uint32_t high= x >> (32 - shift);
    const uint32_t low = x << shift;
    #endif
    return {high, low};
}


__attribute__((always_inline,  optimize( "-Ofast" )))
constexpr fxmath::details::SincosIntermediate DsCosSinPU(uint32_t uq32_x_pu_bits){
    #if 0
    constexpr uint32_t uq32_quatpi_bits = uint32_t(((uint64_t(1u) << 32) / 4) * (M_PI));

    //将一个周期拆分为八个区块 每个区块长度pi/4 获取区块索引
    // const uint32_t sect_num = static_cast<uint32_t>((uq32_x_pu_bits) >> (32 - 3));
    // const uint32_t sect_num = static_cast<uint32_t>((uq32_x_pu_bits) >> (32 - 3));
    uint32_t sect_num;
    std::tie(sect_num, uq32_x_pu_bits) = left_shift_two_part(uq32_x_pu_bits, 3);
    
    //将x由锯齿波变为三角波
    #if 0
    uq32_x_pu_bits = ((sect_num & 0b1)) ? ~uq32_x_pu_bits : uq32_x_pu_bits;
    #else
    const uint32_t inverse_mask = static_cast<uint32_t>(-(int32_t(bool(sect_num & 0b1))));
    // uq32_x_pu_bits = (uq32_x_pu_bits ^ inverse_mask);
    #endif

    #if 1
    //将x继续塌陷 从[0, 2 * pi)变为[0, pi/4) 后期通过诱导公式映射到八个区块的任一区块
    const uint32_t uq32_eeq_x = fxmath::details::mul32hu(uq32_x_pu_bits ^ inverse_mask, uq32_quatpi_bits);
    #endif

    #else
    constexpr uint32_t uq32_quatpi_bits = uint32_t(((uint64_t(1u) << 32) / 4) * (M_PI));

    //将一个周期拆分为八个区块 每个区块长度pi/4 获取区块索引
    const uint32_t sect_num = static_cast<uint32_t>((uq32_x_pu_bits) >> (32 - 3));
    
    //将x由锯齿波变为三角波
    #if 0
    uq32_x_pu_bits = ((sect_num & 0b1)) ? ~uq32_x_pu_bits : uq32_x_pu_bits;
    #else
    const uint32_t inverse_mask = static_cast<uint32_t>(-(int32_t(bool(sect_num & 0b1))));
    uq32_x_pu_bits = (uq32_x_pu_bits ^ inverse_mask);
    #endif

    //将x继续塌陷 从[0, 2 * pi)变为[0, pi/4) 后期通过诱导公式映射到八个区块的任一区块
    const uint32_t uq32_eeq_x = fxmath::details::mul32hu(uq32_x_pu_bits << 3, uq32_quatpi_bits);

    #endif


    #if 1
    //获取每个扇区的偏移值
    const uint32_t uq32_x_offset = (uq32_eeq_x)& 0x03ffffff;

    const uint32_t lut_index = uint32_t(uq32_eeq_x >> 26);


    #else

    // 优化后：一次计算，两次使用
    uint32_t idx_offset = fxmath::details::mul32hu(uq32_x_pu_bits << 3, uq32_quatpi_bits);
    uint32_t lut_index = idx_offset >> 26;        // 高6位
    uint32_t uq32_x_offset = static_cast<uint32_t>(idx_offset << 6) >> 6; // 低26位
    #endif


    #if 1
    __builtin_prefetch(&fxmath::details::IQ31_SINCOS_TABLE[lut_index], 0, 3);
    const auto & pair = fxmath::details::IQ31_SINCOS_TABLE[lut_index];
    const int32_t iq31_sin_coeff = pair[0];
    const int32_t iq31_cos_coeff = pair[1];
    #else
    const int32_t iq31_sin_coeff = fxmath::details::IQ31_SINCOS_TABLE[lut_index][0];
    const int32_t iq31_cos_coeff = fxmath::details::IQ31_SINCOS_TABLE[lut_index][1];
    #endif

    return fxmath::details::SincosIntermediate{
        uq32_x_offset, 
        iq31_sin_coeff,
        iq31_cos_coeff,
        sect_num
    };
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
MAY_INLINE math::fixed<31, int32_t> ds_sinpu2o(const math::fixed<Q, D> x){
    return ds_exact_sin((DsCosSinPU(math::pu_to_uq32(x).to_bits()))
        ,fxmath::details::sincos_exact_laws::taylor_2o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
MAY_INLINE math::fixed<31, int32_t> ds_sinpu3o(const math::fixed<Q, D> x){
    // return ds_exact_sin((DsCosSinPU(math::pu_to_uq32(x).to_bits()))



    return ds_exact_sin((DsCosSinPU(math::pu_to_uq32(x).to_bits()))
        ,fxmath::details::sincos_exact_laws::taylor_3o);

    // return DsCosSinPU(math::pu_to_uq32(x).to_bits())
    //     .exact_sin(fxmath::details::sincos_exact_laws::taylor_3o);

}


template<size_t Q, typename D>
requires (sizeof(D) == 4)
MAY_INLINE math::fixed<31, int32_t> my_sinpu2o(const math::fixed<Q, D> x){
    return math::sinpu_approx(x);
}


template<size_t Q, typename D>
requires (sizeof(D) == 4)
MAY_INLINE math::fixed<31, int32_t> my_cos2o(const math::fixed<Q, D> x){
    return math::cos_approx(x);
}


template<size_t Q, typename D>
requires (sizeof(D) == 4)
MAY_INLINE math::fixed<31, int32_t> ds_sin3o(const math::fixed<Q, D> x){
    return fxmath::details::CosSinPU(rad_to_uq32(x).to_bits())
        .exact_sin(taylor_3o_optimized);
}


template<size_t Q, typename D>
requires (sizeof(D) == 4)
MAY_INLINE math::fixed<31, int32_t> my_sinpu3o(const math::fixed<Q, D> x){
    return math::sinpu(x);
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
        // const auto [s, c] = math::sincospu(x);
        // const auto s = math::sinpu_approx(x);
        // return iq20(math::(x));
        // return iq20((s) + iq20(c));
        // return iq20(math::cospu(x));
        // return iq20(math::sinpu_approx(x));
        // return iq20(s) + iq20(c);
        return ds_sinpu3o(x);
        // return math::sinpu(x);
        // return my_sinpu2o(x);
        // return math::exp(iq16(math::inv_mag(2 * s, c)));
        // return iq16::from_bits(m__IQNdiv_impl<16, true>(s.to_bits(),c.to_bits()));
        // return iq20(s);
    };

    auto fn2 = [](const iq16 x) -> auto {
        // const auto [s, c] = sincospu_approx(x);
        // return math::atan2pu(s,c);

        return my_sinpu3o(x);
        // return 
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


    // for(size_t i = 0; i < 128; i++){
    //     const uq32 x = uq32::from_bits(i * (1u << 23));
    //     // DEBUG_PRINTLN(i, math::sinpu_approx(x).to_bits());
    //     DEBUG_PRINTLN(i, ds_sinpu3o(x).to_bits());
    // }
    // __builtin_trap();
    while(false){
    // while(true){
        // const auto now_secs = clock::seconds_precious();
        static uq32 now_secs = 0;
        now_secs += 0.001_uq32;
        // const auto x = 2 * iq16(frac(now_secs * 2)) * iq16(2 * M_PI) -  1000 * iq16(2 * M_PI);
        // const auto x = iq16(2 * M_PI) * iq16(math::frac(now_secs * 2));
        // const auto x = pu_to_uq32((now_secs * 2));
        const auto x = uq32::from_bits((now_secs * 2).to_bits());
        const auto [s,c] = math::sincospu(x);
        // const auto x = 6 * frac(t * 2) - 3;

        
        // const auto y1 = fn1(s, c);
        // const auto y2 = fn2(s, c);
        // const auto y3 = math::asin(iq16(s));
        // const auto y4 = math::acos(iq16(s));
        // const auto y5 = math::atan2pu(iq16(s), iq16(c));
        const auto y5 = math::atan2(s, c);
        // const auto y5 = math::atanpu(iq16(s) / iq16(c));
        // const auto sx = std::sin(float(math::uq32_to_rad(x)));
        const auto sx = std::sin(double(float(math::uq32_to_rad(x))));
        const auto sx_iq31 = iq31::from(sx);

        const auto f1x = fn1(x);
        const auto f2x = fn2(x);
        const auto elapsed_us = measure_total_elapsed_us([&]{
            formatter.println(
                // uq30(x),
                f1x,
                f2x,
                f1x.to_bits() - f2x.to_bits(),
                sx_iq31.to_bits() - f1x.to_bits(),
                sx_iq31.to_bits() - f2x.to_bits(),
                // f1x.to_bits(),
                // f2x.to_bits(),
                int32_t(sx * (std::numeric_limits<int32_t>::max()) ),
                // std::showpos, 
                // std::showbase,
                // std::hex,
                // uq16(x),
                // uq32(x),
                // (s).to_bits() - fn2(x).to_bits(), 
                // c,
                // s, fn2(x),
                // std::numeric_limits<iq31>::min(), 
                // std::numeric_limits<iq31>::max(), 
                // y1, y2,
                // y3, y4, 
                s,c,
                y5,
                // iq16(s)/iq16(c),
                // iq16::from(float(-y5)),
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
        4096 * 16,
        // 32,
        // +fn1, 
        // +fn2
        fn2,
        fn1
    );
    PANIC{};
}

