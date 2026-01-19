#include "src/testbench/tb.h"

#include "core/math/real.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/clock/clock.hpp"

#include "hal/timer/hw_singleton.hpp"

#include "robots/vendor/mit/cybergear/MotorCyberGear.hpp"

#include "detectors/ShockDetector.hpp"
#include "detectors/SecondOrderTransferFunc.hpp"

#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;

using namespace ymd::robots;

auto input(){
    const auto t = clock::seconds();

    static constexpr auto w0 = real_t(TAU * 4); 
    static constexpr auto w1 = real_t(TAU * 70); 

    static constexpr auto a0 = 1_r; 
    static constexpr auto a1 = 0.07_r; 
    // static constexpr auto a1 = 0.0_r; 

    return a0 * sin(w0 * t) + a1 * sin(w1 * t);
};

void lpf_tb(){
    using Transfer = dsp::LowpassFilter<fixed_t<20>>;

    static constexpr uint ISR_FREQ = 2000;
    const auto config = Transfer::Config{
        .fc = 30,
        .fs = 1000
    };

    Transfer lpf{config};

    hal::timer1.init({ISR_FREQ});

    hal::timer1.attach(TimerIT::Update, {0,0}, [&]{
        const auto x = input();
        lpf.update(x);
        DEBUG_PRINTLN(x, lpf.get());
    });

    while(true);
}


void hpf_tb(){
    using Transfer = dsp::HighpassFilter<fixed_t<20>>;

    static constexpr uint ISR_FREQ = 2000;
    const auto config = Transfer::Config{
        .fc = 30,
        .fs = 1000
    };

    Transfer hpf{config};

    hal::timer1.init({ISR_FREQ});

    hal::timer1.attach(TimerIT::Update, {0,0}, [&]{
        const auto x = input();
        hpf.update(x);
        DEBUG_PRINTLN(x, hpf.result());
    });

    while(true);
}

void bpf_tb(){
    using Bpf = dsp::BandpassFilter<fixed_t<16>>;

    static constexpr uint ISR_FREQ = 2000;
    const auto config = Bpf::Config{
        .fl = 100,
        .fh = 200,
        .fs = 1000
    };

    Bpf bpf{config};

    hal::timer1.init({ISR_FREQ});

    hal::timer1.attach(TimerIT::Update, {0,0}, [&]{
        const auto x = input();
        bpf.update(x);
        DEBUG_PRINTLN(x, bpf.result());
    });

    while(true);
}

void shock_tb(){
    using Transfer = dsp::LowpassFilter<fixed_t<16>>;

    static constexpr uint ISR_FREQ = 2000;
    const auto config = Transfer::Config{
        .fc = 30,
        .fs = ISR_FREQ
    };

    Transfer lpf{config};

    Transfer lpf2{config};

    hal::timer1.init({ISR_FREQ});

    hal::timer1.attach(TimerIT::Update, {0,0}, [&]{
        const auto x = input();
        lpf.update(x);
        lpf2.update(lpf.get());
        // const auto err = x - lpf2.get();
        const auto err = x - lpf.get();
        DEBUG_PRINTLN(x, lpf.get(), err);
    });

    while(true);
}

template<typename Fn>
void dsp_func_test(const uint fs, Fn && fn){

    hal::timer1.init({fs});

    hal::timer1.attach(TimerIT::Update, {0,0}, std::forward<Fn>(fn));

    while(true);
}


void so_tb(){
    // using Sof = dsp::SecondOrderTransferFunc<float>;
    using Sof = dsp::SecondOrderTransferFunc<real_t>;

    // constexpr auto config = Sof::make_butterworth_bpf({

    static constexpr uint fs = 1000;
    const auto config = Sof::make_butterworth_bpf({
        .fl = 20,
        .fh = 70,
        .fs = fs
    });

    Sof sof = {config};
    
    // while(true);
    dsp_func_test(fs, [&](){
        // auto x = input();
        const auto x = clock::seconds();
        // auto r = 0.001_r;
        sof.update(x);
        // DEBUG_PRINTLN(config.a1, config.a2, config.b0, config.b1, config.b2);
        DEBUG_PRINTLN(x, sof.result());
    });
}



void rmst_main(){
    usart2.init({576000});
    DEBUGGER.retarget(&usart2);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");

    // bpf_tb();
    // hpf_tb();
    so_tb();
    // lpf_tb();
    // shock_tb();

    CyberGear motor(can1, 0x01, 0x02);
    while(true){
        // !motor.changeCanId(0);
        
        // !+motor.requestWritePara(0x301, 0x02);
        motor.ctrl({
            0, 0, 0, 500, 5
        }).unwrap();
        clock::delay(10ms);
    }




    std::terminate();
}