
#include "src/testbench/tb.h"
#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"

#include "hal/timer/hw_singleton.hpp"

#include "FFT.hpp"
#include "dsp/filter/butterworth/band.hpp"
#include "dsp/filter/butterworth/side.hpp"
#include "dsp/filter/butterworth/Order4ZeroPhaseShiftButterWothLowpassFilter.hpp"
#include "dsp/siggen/dtmf.hpp"


#include "ParticleSwarmOptimization.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;

using Particle = dsp::Particle<iq16, iq16>;


void dtmf_main(){
    static constexpr uint FS = 8000;
    using DTMF = dsp::DoubleToneMultiFrequencySiggen;
    DTMF dtmf = {{
        .fl_map = {70, 77, 85, 94}, 
        .fh_map = {120, 133, 148, 163}, 
        .fs = FS
    }};

    const iq16 fl = dtmf.fl();
    const iq16 fh = dtmf.fh();
    
    using Filter = dsp::ButterBandpassFilter<iq16, 4>;

    static constexpr auto Qbw = iq16(0.5);

    iq16 side_bw = Qbw * (fh - fl) / 2;

    Filter l_filter {{
        .fs = FS,
        .fl = fl - side_bw,
        .fh = fl + side_bw
    }};

    Filter h_filter = {{
        .fs = FS,
        .fl = fh - side_bw,
        .fh = fh + side_bw
    }};

    {
        dtmf.reset();
        l_filter.reset();
        h_filter.reset();
    }

    auto & timer = hal::timer1;

    timer.init({
        .remap = hal::TIM1_REMAP_A8_A9_A10_A11__B13_B14_B15,
        .count_freq = hal::NearestFreq(FS),
        .count_mode = hal::TimerCountMode::Up
    })            
        .unwrap()
        .alter_to_pins({
            hal::TimerChannelSelection::CH1,
            hal::TimerChannelSelection::CH2,
            hal::TimerChannelSelection::CH3,
        })
        .unwrap();

    timer.start();


    timer.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    timer.enable_interrupt<hal::TimerIT::Update>(EN);
    timer.set_event_handler([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            const auto t = clock::time();
            dtmf.update(t);
            const auto wave = iq16(dtmf.result());

            l_filter.update(wave);
            h_filter.update(wave);

            DEBUG_PRINTLN(
                // CLAMP(wave,0,1),
                t,
                wave,
                h_filter.output(),
                l_filter.output() 
            );
        }
        default: break;
        }
    });

}
