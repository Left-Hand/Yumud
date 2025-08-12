#include "src/testbench/tb.h"
#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"
#include "core/utils/Option.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "FFT.hpp"
#include "liir.hpp"

#include "core/math/realmath.hpp"
#include "types/points/complex.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "func_eval.hpp"
#include <ranges>

using namespace ymd;



namespace ymd::dsp {

template <typename T, size_t N>
struct TwiddleFactors {
    static_assert(std::has_single_bit(N), "N must be a power of 2");
    static constexpr std::array<T, N/2> cos = []{
        std::array<T, N/2> ret;
        for (size_t k = 0; k < N/2; ++k) {
            const T angle = -2 * T(PI) * k / N;
            ret[k] = std::cos(angle);
        }
        return ret;
    }();

    static constexpr std::array<T, N/2> sin = []{
        std::array<T, N/2> ret;
        for (size_t k = 0; k < N/2; ++k) {
            const T angle = -2 * T(PI) * k / N;
            ret[k] = std::sin(angle);
        }
        return ret;
    }();
    
};

template <typename T, size_t N>
constexpr auto twiddle_factors_v = TwiddleFactors<T, N>{};



template <typename T, size_t N>
requires (std::has_single_bit(N))
struct FFT_Impl{
    static constexpr void calc(std::span<Complex<T>> out, std::span<const T> in) noexcept {
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
                out[bitrev[i]].real = in[i];
                out[bitrev[i]].imag = T(0);
            }
        } else {
            // 大规模FFT使用运行时位反转
            for (size_t i = 0; i < N; ++i) {
                out[i].real = in[i];
                out[i].imag = T(0);
            }
            
            for (size_t i = 1, j = 0; i < N - 1; ++i) {
                for (size_t k = N >> 1; k > (j ^= k); k >>= 1);
                if (i < j) {
                    std::swap(out[i].real, out[j].real);
                    std::swap(out[i].imag, out[j].imag);
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
                        
                        const T tw_cos = twiddle_factors_v<T, N>.cos[tw_idx];
                        const T tw_sin = twiddle_factors_v<T, N>.sin[tw_idx];
                        
                        const T temp_real = out[idx2].real * tw_cos - out[idx2].imag * tw_sin;
                        const T temp_imag = out[idx2].real * tw_sin + out[idx2].imag * tw_cos;
                        
                        const T sum_real = out[idx1].real + temp_real;
                        const T sum_imag = out[idx1].imag + temp_imag;
                        const T diff_real = out[idx1].real - temp_real;
                        const T diff_imag = out[idx1].imag - temp_imag;
                        
                        out[idx1].real = sum_real;
                        out[idx1].imag = sum_imag;
                        out[idx2].real = diff_real;
                        out[idx2].imag = diff_imag;
                    }
                }
            } else {
                // 大规模FFT保持循环结构
                for (size_t k = 0; k < N; k += stage) {
                    for (size_t j = 0; j < half; ++j) {
                        const size_t idx1 = k + j;
                        const size_t idx2 = idx1 + half;
                        const size_t tw_idx = j * step;
                        
                        const T tw_cos = twiddle_factors_v<T, N>.cos[tw_idx];
                        const T tw_sin = twiddle_factors_v<T, N>.sin[tw_idx];
                        
                        const T temp_real = out[idx2].real * tw_cos - out[idx2].imag * tw_sin;
                        const T temp_imag = out[idx2].real * tw_sin + out[idx2].imag * tw_cos;
                        
                        out[idx2].real = out[idx1].real - temp_real;
                        out[idx2].imag = out[idx1].imag - temp_imag;
                        out[idx1].real += temp_real;
                        out[idx1].imag += temp_imag;
                    }
                }
            }
        }

        // 3. 实数FFT特有优化：利用共轭对称性
        if constexpr (N > 2) {
            constexpr size_t symm_point = (N >> 1) + 1;
            for (size_t k = symm_point; k < N; ++k) {
                out[k].real = out[N - k].real;
                out[k].imag = -out[N - k].imag;
            }
        }
    }
};

// 特化处理小规模FFT
template <typename T>
struct FFT_Impl<T, 2>{
    static constexpr void calc(std::span<Complex<T>> out,std::span<const T> in) noexcept {
        out[0].real = in[0] + in[1];
        out[1].real = in[0] - in[1];
        out[0].imag = out[1].imag = T(0);
    }
};

// 特化处理小规模FFT
template <typename T>
struct FFT_Impl<T, 4>{
    static constexpr void calc(std::span<Complex<T>> out,std::span<const T> in) noexcept {
        // 位反转排列
        out[0].real = in[0]; out[0].imag = T(0);
        out[1].real = in[2]; out[1].imag = T(0);
        out[2].real = in[1]; out[2].imag = T(0);
        out[3].real = in[3]; out[3].imag = T(0);
        
        // 第一阶段 (N=2)
        {
            T t = out[0].real;
            out[0].real += out[1].real;
            out[1].real = t - out[1].real;
            
            t = out[2].real;
            out[2].real += out[3].real;
            out[3].real = t - out[3].real;
        }
        
        // 第二阶段 (N=4)
        {
            constexpr T w_real = T(0);   // cos(-pi/2)
            constexpr T w_imag = T(-1);  // sin(-pi/2)
            
            T temp_real = out[3].real * w_real - out[3].imag * w_imag;
            T temp_imag = out[3].real * w_imag + out[3].imag * w_real;
            
            out[3].real = out[1].real - temp_real;
            out[3].imag = out[1].imag - temp_imag;
            out[1].real += temp_real;
            out[1].imag += temp_imag;
        }
        
        // 利用对称性
        out[2].real = out[0].real;
        out[2].imag = -out[0].imag;
        out[3].real = out[1].real;
        out[3].imag = -out[1].imag;
    }
};

template <typename T, size_t N>
static constexpr void rfft(std::span<Complex<T>, N> out, std::span<const T, N> in){
    FFT_Impl<T, N>::calc(out, in);
}

} // namespace dsp


#define UART hal::uart2


void fft_main(){
    UART.init({576_KHz});
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets();
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);


    using T = float; 
    // using T = iq_t<16>; 

    [[maybe_unused]]
    // constexpr size_t N = 256;
    constexpr size_t N = 1024;
    // constexpr size_t N = 512;

    [[maybe_unused]] auto make_sin_samples = []() -> std::array<T, N>{
        std::array<T, N> ret;
        const auto freq = q16(256.97) / N;
        for(size_t i = 0; i < N; ++i){
            ret[i] = T(sinpu(freq * i));
        }
        return ret;
    };

    [[maybe_unused]] auto make_am_samples = []() -> std::array<T, N>{
        std::array<T, N> ret;
        constexpr auto modu_freq = q16(8.97) / N;
        constexpr auto carry_freq = q16(56.97) / N;
        constexpr auto modu_depth = 0.4_r;
        for(size_t i = 0; i < N; ++i){
            ret[i] = T(sinpu(carry_freq * i) * (1 + modu_depth * sinpu(modu_freq * i)));
        }
        return ret;
    };

    // const auto samples = make_sin_samples();
    const auto samples = make_am_samples();
    std::array<Complex<T>, N> out;

    const auto elapsed_us = measure_total_elapsed_ms([&]{
        dsp::rfft(
            std::span(out), 
            std::span(samples)
        );
    });
    // DEBUG_PRINTLN(samples);
    // DEBUG_PRINTLN(out.real, out.imag);

    for(size_t i = 0; i < N; i++){
        DEBUG_PRINTLN(
            out[i], 
            samples[i],
            out[i].abs()
            // out | std::views::transform([](const Complex<auto> & x){return x.abs();})
        );
    }

    DEBUG_PRINTLN(elapsed_us);

    while(true){
        // DEBUG_PRINTLN(clock::millis());
        clock::delay(5ms);
    }
}