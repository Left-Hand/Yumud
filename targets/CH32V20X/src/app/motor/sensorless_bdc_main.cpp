#include "src/testbench/tb.h"

#include "core/math/realmath.hpp"
#include "core/system.hpp"
#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/timer.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "hal/analog/adc/adcs/adc1.hpp"
#include "hal/bus/uart/uartsw.hpp"


#include "dsp/filter/butterworth/ButterSideFilter.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"

#include "dsp/filter/homebrew/debounce_filter.hpp"
#include "dsp/controller/adrc/tracking_differentiator.hpp"
#include "dsp/homebrew/edge_counter.hpp"
#include "dsp/controller/smc/sliding_mode_ctrl.hpp"

#include "digipw/ctrl/pi_controller.hpp"

using namespace ymd;


static constexpr size_t ISR_FREQ = 19200 * 2;
static constexpr real_t SAMPLE_RES = 0.008_r;
static constexpr real_t INA240_BETA = 100;
static constexpr real_t VOLT_BAIS = 1.65_r;



real_t volt_2_current(real_t volt){
    static constexpr auto INV_SCALE = 1 / (SAMPLE_RES * INA240_BETA);
    return (volt - VOLT_BAIS) *INV_SCALE;
}


using LowpassFilter = dsp::ButterLowpassFilter<q16, 2>;
using HighpassFilter = dsp::ButterHighpassFilter<q16, 2>;
using BandpassFilter = dsp::ButterBandpassFilter<q16, 4>;

[[maybe_unused]] static 
void at8222_tb(){

    hal::uart2.init({4000000, CommStrategy::Nil});

    DEBUGGER.retarget(&hal::uart2);
    DEBUGGER.no_brackets(EN);


    auto & timer = hal::timer3;

    //因为是中心对齐的顶部触发 所以频率翻�?
    timer.init({
        .count_freq = hal::NearestFreq(ISR_FREQ * 2),
        .count_mode = hal::TimerCountMode::CenterAlignedUpTrig
    }, EN);

    auto & pwm_pos = timer.oc<1>();
    auto & pwm_neg = timer.oc<2>();

    
    pwm_pos.init({});
    pwm_neg.init({});


    hal::adc1.init(
        {
            {hal::AdcChannelSelection::VREF, hal::AdcSampleCycles::T28_5}
        },{
            // {hal::AdcChannelSelection::CH4, hal::AdcSampleCycles::T28_5},
            {hal::AdcChannelSelection::CH4, hal::AdcSampleCycles::T28_5},
        }, {}
    );

    hal::adc1.set_injected_trigger(hal::AdcInjectedTrigger::T3CC4);
    hal::adc1.enable_auto_inject(DISEN);
    // hal::adc1.set_pga(AdcPga::X16);

    timer.set_trgo_source(hal::TimerTrgoSource::OC4R);

    timer.oc<4>().init({.plant_en = DISEN});

    // timer.oc(4).cvr() = timer.arr() - 1; 
    // timer.oc(4).cvr() = int(timer.arr() * 0.1_r); 
    timer.oc<4>().cvr() = int(1); 
    
    LowpassFilter lpf{LowpassFilter::Config{
        .fc = 640,
        .fs = ISR_FREQ
    }};

    
    LowpassFilter lpf_mid{LowpassFilter::Config{
        .fc = 200,
        .fs = ISR_FREQ
    }};

    BandpassFilter bpf{BandpassFilter::Config{
        // .fl = 300,
        // .fh = 800,
        .fs = ISR_FREQ,
        .fl = 150,
        .fh = 400
    }};


    dsp::EdgeCounter ect;
    
    real_t curr = 0;
    [[maybe_unused]]real_t curr_mid = 0;

    // IController pi_ctrl{
    //     IController::Config{
    //         .ki = 0.4_r,
    //         .out_min = 0.7_r,
    //         .out_max = 0.97_r,
    //         .fs = ISR_FREQ
    //     }
    // };

    // myPIController pi_ctrl{
    //     myPIController::Config{
    //         .kp = 0.0_r,
    //         .ki = 0.2_r,
    //         .out_min = 0.7_r,
    //         .out_max = 0.97_r,
    //         .fs = ISR_FREQ
    //     }
    // };

    // dsp::SlidingModeController pi_ctrl{{
    //     .c = 0.16_q24,
    //     .q = 0.0005_q24,
    //     // .c = 01.6_q24,
    //     // .q = 0.0000005_q24,

    //     .out_min = 0.7_r,
    //     .out_max = 0.97_r,

    //     .fs = ISR_FREQ
    // }};
    
    // digipw::PiController pi_ctrl{{
    //     .fs = ISR_FREQ
    //     .kp = 0.01_r,
    //     .ki = 0.01_r,
    //     .out_min = 0.7_r,
    //     .out_max = 0.97_r,
    // }};

    dsp::TrackingDifferentiatorByOrders<2> td{{
        // .r = 14.96_r,
        // .r = 7.9_r,
        // .r = 7.99_r,
        // .r = 48.00_r,
        // .r = 38.00_r,
        .r = 125.5_r,
        // .r = 6.5_r,
        .fs = ISR_FREQ
    }};

    // volatile uint32_t exe_micros = 0;
    real_t spd_targ = 0;
    real_t pos_targ = 0;

    real_t trackin_sig = 0;
    real_t volt = 0;

    auto watch_gpio = hal::PA<3>();
    watch_gpio.outpp();


    hal::adc1.register_nvic({0,0}, EN);
    hal::adc1.enable_interrupt<hal::AdcIT::JEOC>(EN);
    hal::adc1.set_event_callback(
        [&](const hal::AdcEvent ev){
            switch(ev){
            case hal::AdcEvent::EndOfInjectedConversion:{
                watch_gpio.toggle();
                volt = hal::adc1.inj<1>().get_voltage();
                const auto curr_raw = volt_2_current(volt);

                lpf.update(curr_raw);
                lpf_mid.update(curr_raw);
                // curr = lpf.get();
                curr = curr_raw;
                watch_gpio.toggle();
                // bpf.update(curr_raw);

                bpf.update(curr_raw);
                curr_mid = lpf_mid.get();

                ect.update(bool(bpf.output() > 0));

                const auto pos = ect.count() * 0.01_r;
                td.update(pos);
                [[maybe_unused]] const auto spd = td.state()[1];

                // static constexpr auto kp = 267.0_r;
                // static constexpr auto kd = 0.0_r;
                // const auto spd_cmd = kp * (pos_targ - pos) + kd * (spd_targ - spd);
                // pi_ctrl.update(spd_targ, spd);
                // pwm_pos = pi_ctrl.get();
                // pwm_pos = 0.87_r * abs(sinpu(time()));
                // pwm_pos = 0.7_r + 0.17_r * abs(sinpu(time()));
                // pwm_pos = 0.13_r + 0.817_r * abs(sinpu(time()));
                pwm_pos.set_dutycycle(0_r);
                // pwm_neg = LERP(0.32_r, 0.32_r, sin(time()) * 0.5_r + 0.5_r);
                // pwm_neg = LERP(0.32_r, 0.32_r, sin(time()) * 0.5_r + 0.5_r);
                pwm_neg.set_dutycycle(0.9_r);
                break;}
            default: break;
            }
        }
    );



    hal::TimerOcPair motdrv{
        pwm_pos, pwm_neg
    };


    while(true){
        // const auto duty = sin(4 * time()) * 0.2_r + 0.7_r;
        // motdrv = duty;
        // pwm_neg = 0.5_r;
        // pwm_pos = ABS(duty);

        // pwm_pos = 0.9_r + 0.1_r * sin(5 * time());
        // spd_targ = 7.0_r + 3 * sin(5 * time());
        // spd_targ = 8.0_r + 1.0_r * ((sin(2.0_r * time())) > 0 ? 1 : -1);
        // spd_targ = 8.0_r + 1.0_r * sinpu(2.0_r * time());
        const auto ctime = clock::time();

        #define TEST_MODE 1

        #if TEST_MODE == 0
        spd_targ = 12;
        pos_targ = 10.0_r * ctime + 2*frac(ctime);
        #elif TEST_MODE == 1
        spd_targ = 7.0_r + 1.0_r * sinpu(1.3_r * ctime);
        pos_targ = 7.0_r * ctime + real_t(-1.0/6) * cospu(1.3_r * ctime);
        #endif
        // spd_targ = 9.0_r + 1.0_r * ((sin(1.0_r * time())) > 0 ? 1 : ;
        // spd_targ = 9.0_r + 1.0_r * -1;
        // spd_targ = 16.57_r;
        // trackin_sig = sign(sin(ctime * 3));
        // trackin_sig = real_t(int(sin(ctime * 3) * 32)) / 32;
        // trackin_sig = real_t(int(0.2_r * sin(ctime * 3) * 32)) / 32;
        // trackin_sig = real_t(int(0.2_r * ctime * 32)) / 32;
        // trackin_sig = 1/(1 + exp(4 * tpzpu(3 * ctime)));
        trackin_sig = 10 * CLAMP2(sinpu(7 * ctime), 0.5_r);
        // trackin_sig = tpzpu(t);
        
        // DEBUG_PRINTLN_IDLE(pos_targ, spd_targ, bpf.get(), volt, pi_ctrl.get(), bpf.get(), , exe_micros);
        // DEBUG_PRINTLN_IDLE(td.get(), lpf.get() * 90 ,volt,spd_targ, exe_micros);
        DEBUG_PRINTLN_IDLE(curr * 1000,curr_mid * 1000, volt, curr > curr_mid);
        // DEBUG_PRINTLN_IDLE(trackin_sig, td.get());
        // DEBUG_PRINTLN(bool(pwm_pos.io()), bool(pwm_neg.io()));
        
    }
}
