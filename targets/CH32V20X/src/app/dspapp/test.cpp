#include "src/testbench/tb.h"

#include "sys/clock/time.hpp"

#include "FFT.hpp"
#include "liir.hpp"
#include "sys/debug/debug.hpp"
#include "ButterBandFilter.hpp"

#include "hal/timer/instance/timer_hw.hpp"


namespace ymd::dsp{
    template<typename FnIn, typename FnFt>
    void evaluate_func(const uint times, FnIn && fn_in, FnFt && fn_ft){
        
        const auto begin_m = micros();

        for(size_t i = 0; i < times; ++i){
            const auto x = real_t(i) / times;
            std::forward<FnFt>(fn_ft)(x);
        }

        const auto end_m = micros();
        
        DEBUG_PRINTLN(real_t(uint32_t(end_m - begin_m)) / times, "us per call");
        delay(20);
        std::terminate();
    }

    template<typename FnIn, typename FnFt>
    void run_func(const uint fs, FnIn && fn_in, FnFt && fn_ft){
        
        hal::timer1.init(fs);
        
        hal::timer1.attach(TimerIT::Update, {0,0}, [&](){
            const auto x = std::forward<FnIn>(fn_in)();
            const auto y = std::forward<FnFt>(fn_ft)(x);

            DEBUG_PRINTLN(x, y);
        });
        
        while(true);
    }

    namespace samples{
        template<arithmetic T>
        T sinwave(const T t){
            return std::sin(t);
        }
    }
}

using namespace ymd;
// using namespace ymd::dsp;

template<typename T, size_t n>
void butterworth_bandpass_coeff_tb(const T f1f, const T f2f, const bool scale_en = false){

    // n = order of the filter
    // fc1 = lower cutoff frequency as a fraction of Pi [0,1]
    // fc2 = upper cutoff frequency as a fraction of Pi [0,1]
    // sf = 1 to scale c coefficients for normalized response
    // sf = 0 to not scale c coefficients

    /* calculate the c coefficients */
    auto ccof = dsp::ccof_bwbp<n>();

    /* calculate the d coefficients */
    auto dcof = dsp::dcof_bwbp<T, n>(f1f, f2f );

    // const T sf = dsp::sf_bwbp<T, n>(f1f, f2f ); /* scaling factor for the c coefficients */

    DEBUG_PRINTLN(ccof);
    delay(20);
    DEBUG_PRINTLN(dcof);
    delay(20);

    std::terminate();
}

template<typename T, size_t n>
void butterworth_bandpass_tb(auto && fn_in, const T fl, const T fh, const uint fs){
    using Filter = dsp::ButterBandpassFilter<T, n>;
    using Config = Filter::Config;

    Filter filter = {Config{
        .fl = fl,
        .fh = fh,
        .fs = fs,
    }};

    // dsp::run_func(fs, fn_in, filter);
    dsp::evaluate_func(fs, fn_in, filter);
}
 


void dsp_main(){
    uart2.init(576000);
    DEBUGGER.retarget(&uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");


    // using T = float; 
    using T = iq_t<16>; 

    constexpr T fl = T(50);
    constexpr T fh = T(200);
    constexpr uint fs = 2000;

    constexpr size_t n = 12;

    auto sig_in = [&]() {
        const T t = 120 * T(TAU) * T(time());
        return dsp::samples::sinwave(t);
    };

    butterworth_bandpass_tb<T, n>(sig_in, fl, fh, fs);


    {
        // /* calculate the d coefficients */
        // static constexpr auto dcof = dsp::dcof_bwbp<std::decay_t<decltype(f1f)>, n>(f1f, f2f );

        // /* calculate the c coefficients */
        // static constexpr auto ccof = dsp::ccof_bwbp<n>();
    }
}