
#include "src/testbench/tb.h"
#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"

#include "hal/timer/instance/timer_hw.hpp"

#include "FFT.hpp"
#include "liir.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"
#include "dsp/filter/butterworth/ButterSideFilter.hpp"
#include "dsp/filter/butterworth/Order4ZeroPhaseShiftButterWothLowpassFilter.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/siggen/dtmf.hpp"


#include "ParticleSwarmOptimization.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;

using Particle = dsp::Particle<real_t, real_t>;


void dtmf_main(){
    static constexpr uint fs = 8000;
    using DTMF = DoubleToneMultiFrequencySiggen;
    DTMF dtmf = {{
        .fl_map = {70, 77, 85, 94}, 
        .fh_map = {120, 133, 148, 163}, 
        .fs = fs
    }};

    const real_t fl = dtmf.fl();
    const real_t fh = dtmf.fh();
    
    using Filter = dsp::ButterBandpassFilter<real_t, 4>;

    static constexpr auto Qbw = real_t(0.5);

    real_t side_bw = Qbw * (fh - fl) / 2;

    Filter l_filter {{
        .fl = fl - side_bw,
        .fh = fl + side_bw,
        .fs = fs
    }};

    Filter h_filter = {{
        .fl = fh - side_bw,
        .fh = fh + side_bw,
        .fs = fs
    }};

    {
        dtmf.reset();
        l_filter.reset();
        h_filter.reset();
    }

    hal::timer1.init({fs});
    hal::timer1.attach(hal::TimerIT::Update, {0,0}, [&](){
        const auto t = clock::time();
        dtmf.update(t);
        const auto wave = real_t(dtmf.result());

        l_filter.update(wave);
        h_filter.update(wave);

        DEBUG_PRINTLN(
            // CLAMP(wave,0,1),
            t,
            wave,
            h_filter.get(),
            l_filter.get() 
        );
    });
}
