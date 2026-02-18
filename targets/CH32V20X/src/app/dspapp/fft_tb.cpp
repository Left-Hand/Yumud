#include "src/testbench/tb.h"
#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"
#include "core/utils/Option.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "FFT.hpp"

#include "core/math/realmath.hpp"
#include "algebra/vectors/complex.hpp"

#include "hal/bus/uart/hw_singleton.hpp"
#include "func_eval.hpp"
#include <ranges>

using namespace ymd;



namespace ymd::dsp {

template <typename T, size_t N>
struct TwiddleFactors {
    static_assert(std::has_single_bit(N), "N must be a power of 2");
    static constexpr std::array<T, N/2> COS_TABLE = []{
        std::array<T, N/2> ret;
        for (size_t k = 0; k < N/2; ++k) {
            const T angle = -2 * T(M_PI) * k / N;
            ret[k] = std::cos(angle);
        }
        return ret;
    }();

    static constexpr std::array<T, N/2> SIN_TABLE = []{
        std::array<T, N/2> ret;
        for (size_t k = 0; k < N/2; ++k) {
            const T angle = -2 * T(M_PI) * k / N;
            ret[k] = std::sin(angle);
        }
        return ret;
    }();
    
};



template <typename T, size_t N>
requires (std::has_single_bit(N))
struct FFT_Impl{
    static constexpr void calc(std::span<math::Complex<T>> dst, std::span<const T> src) noexcept {
        static_assert(N >= 2, "FFT size must be at least 2");
        
        // 1. 初始化并位反转置换
        if constexpr (N <= 64) {
            // 小规模FFT使用编译时位反转表
            constexpr auto bitrev = []() {
                std::array<size_t, N> table{};
                for (size_t i = 0; i < N; ++i) {
                    size_t j = 0;
                    for (size_t mask = 1, rev_mask = N/2; mask < N; mask <<= 1, rev_mask >>= 1) {
                        if (i & mask) j |= rev_mask;
                    }
                    table[i] = j;
                }
                return table;
            }();
            
            for (size_t i = 0; i < N; ++i) {
                dst[bitrev[i]].re = src[i];
                dst[bitrev[i]].im = T(0);
            }
        } else {
            // 大规模FFT使用运行时位反转
            for (size_t i = 0; i < N; ++i) {
                dst[i].re = src[i];
                dst[i].im = T(0);
            }
            
            for (size_t i = 1, j = 0; i < N - 1; ++i) {
                for (size_t k = N >> 1; k > (j ^= k); k >>= 1);
                if (i < j) {
                    std::swap(dst[i].re, dst[j].re);
                    std::swap(dst[i].im, dst[j].im);
                }
            }
        }

        // 2. 分阶段计算FFT
        for (size_t stage = 2; stage <= N; stage <<= 1) {
            const size_t half = stage >> 1;
            const size_t step = N / stage;
            
            if constexpr (N <= 1024) {
                // 小规模FFT展开内部循环
                for (size_t group = 0; group < N; group += stage) {
                    for (size_t pair = 0; pair < half; ++pair) {
                        const size_t idx1 = group + pair;
                        const size_t idx2 = idx1 + half;
                        const size_t tw_idx = pair * step;
                        
                        const T tw_cos = TwiddleFactors<T, N>::COS_TABLE[tw_idx];
                        const T tw_sin = TwiddleFactors<T, N>::SIN_TABLE[tw_idx];
                        
                        const T temp_re = dst[idx2].re * tw_cos - dst[idx2].im * tw_sin;
                        const T temp_im = dst[idx2].re * tw_sin + dst[idx2].im * tw_cos;
                        
                        const T sum_re = dst[idx1].re + temp_re;
                        const T sum_im = dst[idx1].im + temp_im;
                        const T diff_re = dst[idx1].re - temp_re;
                        const T diff_im = dst[idx1].im - temp_im;
                        
                        dst[idx1].re = sum_re;
                        dst[idx1].im = sum_im;
                        dst[idx2].re = diff_re;
                        dst[idx2].im = diff_im;
                    }
                }
            } else {
                // 大规模FFT保持循环结构
                for (size_t k = 0; k < N; k += stage) {
                    for (size_t j = 0; j < half; ++j) {
                        const size_t idx1 = k + j;
                        const size_t idx2 = idx1 + half;
                        const size_t tw_idx = j * step;
                        
                        const T tw_cos = TwiddleFactors<T, N>::COS_TABLE[tw_idx];
                        const T tw_sin = TwiddleFactors<T, N>::SIN_TABLE[tw_idx];
                        
                        const T temp_re = dst[idx2].re * tw_cos - dst[idx2].im * tw_sin;
                        const T temp_im = dst[idx2].re * tw_sin + dst[idx2].im * tw_cos;
                        
                        dst[idx2].re = dst[idx1].re - temp_re;
                        dst[idx2].im = dst[idx1].im - temp_im;
                        dst[idx1].re += temp_re;
                        dst[idx1].im += temp_im;
                    }
                }
            }
        }

        // 3. 实数FFT特有优化：利用共轭对称性
        if constexpr (N > 2) {
            constexpr size_t symm_point = (N >> 1) + 1;
            for (size_t k = symm_point; k < N; ++k) {
                dst[k].re = dst[N - k].re;
                dst[k].im = -dst[N - k].im;
            }
        }
    }
};

// 特化处理小规模FFT
template <typename T>
struct FFT_Impl<T, 2>{
    static constexpr void calc(std::span<math::Complex<T>> dst,std::span<const T> src) noexcept {
        dst[0].re = src[0] + src[1];
        dst[1].re = src[0] - src[1];
        dst[0].im = dst[1].im = T(0);
    }
};

// 特化处理小规模FFT
template <typename T>
struct FFT_Impl<T, 4>{
    static constexpr void calc(std::span<math::Complex<T>> dst,std::span<const T> src) noexcept {
        // 位反转排列
        dst[0].re = src[0]; dst[0].im = T(0);
        dst[1].re = src[2]; dst[1].im = T(0);
        dst[2].re = src[1]; dst[2].im = T(0);
        dst[3].re = src[3]; dst[3].im = T(0);
        
        // 第一阶段 (N=2)
        {
            T t = dst[0].re;
            dst[0].re += dst[1].re;
            dst[1].re = t - dst[1].re;
            
            t = dst[2].re;
            dst[2].re += dst[3].re;
            dst[3].re = t - dst[3].re;
        }
        
        // 第二阶段 (N=4)
        {
            constexpr T w_re = T(0);   // cos(-pi/2)
            constexpr T w_im = T(-1);  // sin(-pi/2)
            
            T temp_re = dst[3].re * w_re - dst[3].im * w_im;
            T temp_im = dst[3].re * w_im + dst[3].im * w_re;
            
            dst[3].re = dst[1].re - temp_re;
            dst[3].im = dst[1].im - temp_im;
            dst[1].re += temp_re;
            dst[1].im += temp_im;
        }
        
        // 利用对称性
        dst[2].re = dst[0].re;
        dst[2].im = -dst[0].im;
        dst[3].re = dst[1].re;
        dst[3].im = -dst[1].im;
    }
};

template <size_t N, typename T>
static constexpr void rfft(std::span<math::Complex<T>> dst, std::span<const T> src){
    FFT_Impl<T, N>::calc(dst, src);
}

} // namespace dsp


#define UART hal::usart2


void fft_main(){
    UART.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
    });
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);


    using T = float; 
    // using T = fixed<16>; 

    [[maybe_unused]]
    // constexpr size_t N = 256;
    constexpr size_t N = 1024;
    // constexpr size_t N = 512;

    [[maybe_unused]] auto make_sine_samples = []() -> std::vector<T>{
        std::vector<T> ret;
        const auto freq = iq16(256.97) / N;
        for(size_t i = 0; i < N; ++i){
            ret.push_back(T(math::sinpu(freq * i)));
        }
        return ret;
    };

    [[maybe_unused]] auto make_am_samples = []() -> std::vector<T>{
        std::vector<T> ret;
        constexpr auto modu_freq = iq16(8.97) / N;
        constexpr auto carry_freq = iq16(56.97) / N;
        constexpr auto modu_depth = 0.4_r;
        for(size_t i = 0; i < N; ++i){
            ret.push_back(T(math::sinpu(carry_freq * i) * (1 + modu_depth * math::sinpu(modu_freq * i))));
        }
        return ret;
    };

    // const auto samples = make_sine_samples();
    const auto samples = make_am_samples();
    std::vector<math::Complex<T>> dst;
    dst.reserve(samples.size());

    const auto elapsed_us = measure_total_elapsed_ms([&]{
        dsp::rfft<N>(
            std::span(dst), 
            std::span(samples)
        );
    });
    // DEBUG_PRINTLN(samples);
    // DEBUG_PRINTLN(dst.re, dst.im);

    for(size_t i = 0; i < N; i++){
        DEBUG_PRINTLN(
            dst[i], 
            samples[i],
            dst[i].length()
            // dst | std::views::transform([](const math::Complex<auto> & x){return x.abs();})
        );
    }

    DEBUG_PRINTLN(elapsed_us);

    while(true){
        // DEBUG_PRINTLN(clock::millis());
        clock::delay(5ms);
    }
}