#include "src/testbench/tb.h"

#include "FFT.hpp"
#include "liir.hpp"
#include "sys/debug/debug.hpp"

using namespace ymd;
// using namespace ymd::dsp;
 

template<typename T, size_t n>
void butterworth_bandpass_tb(const T f1f, const T f2f, const bool scale_en = false){

    // n = order of the filter
    // fc1 = lower cutoff frequency as a fraction of Pi [0,1]
    // fc2 = upper cutoff frequency as a fraction of Pi [0,1]
    // sf = 1 to scale c coefficients for normalized response
    // sf = 0 to not scale c coefficients

    /* calculate the d coefficients */
    auto dcof = dsp::dcof_bwbp<T, n>(f1f, f2f );

    /* calculate the c coefficients */
    auto ccof = dsp::ccof_bwbp<n>();

    // const T sf = dsp::sf_bwbp<T, n>(f1f, f2f ); /* scaling factor for the c coefficients */

    DEBUG_PRINTLN(ccof);
    delay(20);
    DEBUG_PRINTLN(dcof);
    delay(20);

    std::terminate();
 }
 


void dsp_main(){
    uart2.init(576000);
    DEBUGGER.retarget(&uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");

    constexpr size_t n = 1;
    constexpr real_t f1f = 0.2_r;
    constexpr real_t f2f = 0.8_r;
    butterworth_bandpass_tb<real_t, n>(f1f, f2f);

    {
        // /* calculate the d coefficients */
        // static constexpr auto dcof = dsp::dcof_bwbp<std::decay_t<decltype(f1f)>, n>(f1f, f2f );

        // /* calculate the c coefficients */
        // static constexpr auto ccof = dsp::ccof_bwbp<n>();
    }
}