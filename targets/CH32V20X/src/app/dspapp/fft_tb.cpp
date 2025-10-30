#include "src/testbench/tb.h"
#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"
#include "core/utils/Option.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "FFT.hpp"
#include "liir.hpp"

#include "core/math/realmath.hpp"
#include "types/vectors/complex.hpp"

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
    static constexpr void calc(std::span<Complex<T>> dst, std::span<const T> src) noexcept {
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
                dst[bitrev[i]].real = src[i];
                dst[bitrev[i]].imag = T(0);
            }
        } else {
            // 大规模FFT使用运行时位反转
            for (size_t i = 0; i < N; ++i) {
                dst[i].real = src[i];
                dst[i].imag = T(0);
            }
            
            for (size_t i = 1, j = 0; i < N - 1; ++i) {
                for (size_t k = N >> 1; k > (j ^= k); k >>= 1);
                if (i < j) {
                    std::swap(dst[i].real, dst[j].real);
                    std::swap(dst[i].imag, dst[j].imag);
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
                        
                        const T temp_real = dst[idx2].real * tw_cos - dst[idx2].imag * tw_sin;
                        const T temp_imag = dst[idx2].real * tw_sin + dst[idx2].imag * tw_cos;
                        
                        const T sum_real = dst[idx1].real + temp_real;
                        const T sum_imag = dst[idx1].imag + temp_imag;
                        const T diff_real = dst[idx1].real - temp_real;
                        const T diff_imag = dst[idx1].imag - temp_imag;
                        
                        dst[idx1].real = sum_real;
                        dst[idx1].imag = sum_imag;
                        dst[idx2].real = diff_real;
                        dst[idx2].imag = diff_imag;
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
                        
                        const T temp_real = dst[idx2].real * tw_cos - dst[idx2].imag * tw_sin;
                        const T temp_imag = dst[idx2].real * tw_sin + dst[idx2].imag * tw_cos;
                        
                        dst[idx2].real = dst[idx1].real - temp_real;
                        dst[idx2].imag = dst[idx1].imag - temp_imag;
                        dst[idx1].real += temp_real;
                        dst[idx1].imag += temp_imag;
                    }
                }
            }
        }

        // 3. 实数FFT特有优化：利用共轭对称性
        if constexpr (N > 2) {
            constexpr size_t symm_point = (N >> 1) + 1;
            for (size_t k = symm_point; k < N; ++k) {
                dst[k].real = dst[N - k].real;
                dst[k].imag = -dst[N - k].imag;
            }
        }
    }
};

// 特化处理小规模FFT
template <typename T>
struct FFT_Impl<T, 2>{
    static constexpr void calc(std::span<Complex<T>> dst,std::span<const T> src) noexcept {
        dst[0].real = src[0] + src[1];
        dst[1].real = src[0] - src[1];
        dst[0].imag = dst[1].imag = T(0);
    }
};

// 特化处理小规模FFT
template <typename T>
struct FFT_Impl<T, 4>{
    static constexpr void calc(std::span<Complex<T>> dst,std::span<const T> src) noexcept {
        // 位反转排列
        dst[0].real = src[0]; dst[0].imag = T(0);
        dst[1].real = src[2]; dst[1].imag = T(0);
        dst[2].real = src[1]; dst[2].imag = T(0);
        dst[3].real = src[3]; dst[3].imag = T(0);
        
        // 第一阶段 (N=2)
        {
            T t = dst[0].real;
            dst[0].real += dst[1].real;
            dst[1].real = t - dst[1].real;
            
            t = dst[2].real;
            dst[2].real += dst[3].real;
            dst[3].real = t - dst[3].real;
        }
        
        // 第二阶段 (N=4)
        {
            constexpr T w_real = T(0);   // cos(-pi/2)
            constexpr T w_imag = T(-1);  // sin(-pi/2)
            
            T temp_real = dst[3].real * w_real - dst[3].imag * w_imag;
            T temp_imag = dst[3].real * w_imag + dst[3].imag * w_real;
            
            dst[3].real = dst[1].real - temp_real;
            dst[3].imag = dst[1].imag - temp_imag;
            dst[1].real += temp_real;
            dst[1].imag += temp_imag;
        }
        
        // 利用对称性
        dst[2].real = dst[0].real;
        dst[2].imag = -dst[0].imag;
        dst[3].real = dst[1].real;
        dst[3].imag = -dst[1].imag;
    }
};

template <size_t N, typename T>
static constexpr void rfft(std::span<Complex<T>> dst, std::span<const T> src){
    FFT_Impl<T, N>::calc(dst, src);
}

} // namespace dsp


#define UART hal::uart2


void fft_main(){
    UART.init({576_KHz});
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);


    using T = float; 
    // using T = fixed_t<16>; 

    [[maybe_unused]]
    // constexpr size_t N = 256;
    constexpr size_t N = 1024;
    // constexpr size_t N = 512;

    [[maybe_unused]] auto make_sin_samples = []() -> std::vector<T>{
        std::vector<T> ret;
        const auto freq = iq16(256.97) / N;
        for(size_t i = 0; i < N; ++i){
            ret.push_back(T(sinpu(freq * i)));
        }
        return ret;
    };

    [[maybe_unused]] auto make_am_samples = []() -> std::vector<T>{
        std::vector<T> ret;
        constexpr auto modu_freq = iq16(8.97) / N;
        constexpr auto carry_freq = iq16(56.97) / N;
        constexpr auto modu_depth = 0.4_r;
        for(size_t i = 0; i < N; ++i){
            ret.push_back(T(sinpu(carry_freq * i) * (1 + modu_depth * sinpu(modu_freq * i))));
        }
        return ret;
    };

    // const auto samples = make_sin_samples();
    const auto samples = make_am_samples();
    std::vector<Complex<T>> dst;
    dst.reserve(samples.size());

    const auto elapsed_us = measure_total_elapsed_ms([&]{
        dsp::rfft<N>(
            std::span(dst), 
            std::span(samples)
        );
    });
    // DEBUG_PRINTLN(samples);
    // DEBUG_PRINTLN(dst.real, dst.imag);

    for(size_t i = 0; i < N; i++){
        DEBUG_PRINTLN(
            dst[i], 
            samples[i],
            dst[i].length()
            // dst | std::views::transform([](const Complex<auto> & x){return x.abs();})
        );
    }

    DEBUG_PRINTLN(elapsed_us);

    while(true){
        // DEBUG_PRINTLN(clock::millis());
        clock::delay(5ms);
    }
}