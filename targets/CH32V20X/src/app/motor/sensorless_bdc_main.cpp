#include "src/testbench/tb.h"

#include "core/math/realmath.hpp"
#include "core/system.hpp"
#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/timer.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/uart/uartsw.hpp"


#include "drivers/Actuator/Bridge/AT8222/at8222.hpp"
#include "dsp/filter/butterworth/ButterSideFilter.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"

#include "dsp/filter/homebrew/DigitalFilter.hpp"
#include "dsp/controller/adrc/tracking_differentiator.hpp"
#include "dsp/controller/pi_ctrl.hpp"
#include "dsp/homebrew/edge_counter.hpp"
#include "dsp/controller/smc/sliding_mode_ctrl.hpp"


static constexpr size_t ISR_FREQ = 19200 * 2;
static constexpr real_t SAMPLE_RES = 0.008_r;
static constexpr real_t INA240_BETA = 100;
static constexpr real_t VOLT_BAIS = 1.65_r;

template<size_t Q>
static constexpr iq_t<Q> tpzpu(const iq_t<Q> x){
    return abs(4 * frac(x - iq_t<Q>(0.25)) - 2) - 1;
}

real_t volt_2_current(real_t volt){
    static constexpr auto INV_SCALE = 1 / (SAMPLE_RES * INA240_BETA);
    return (volt - VOLT_BAIS) *INV_SCALE;
}



using LowpassFilter = dsp::ButterLowpassFilter<q16, 2>;
using HighpassFilter = dsp::ButterHighpassFilter<q16, 2>;
using BandpassFilter = dsp::ButterBandpassFilter<q16, 4>;






[[maybe_unused]] static 
void at8222_tb(){
    hal::UartSw uart{portA[5], NullGpio}; uart.init(19200);
    // DEBUGGER.retarget(&uart);

    uart2.init(4000000, CommStrategy::Nil);
    // uart2.init(921600);
    // while(true){
    //     // uart2.write1(0x55);
    // }
    // DEBUGGER.retarget(&uart2);
    DEBUGGER.retarget(&uart);
    DEBUGGER.no_brackets();

    // while(true){
    //     DEBUG_PRINTLN('/');
    // }

    // TARG_UART.init(6_MHz);

    auto & timer = hal::timer3;

    //因为是中心对齐的顶部触发 所以频率翻�?
    timer.init(ISR_FREQ * 2, TimerCountMode::CenterAlignedUpTrig);

    auto & pwm_pos = timer.oc(1);
    auto & pwm_neg = timer.oc(2);

    
    pwm_pos.init({});
    pwm_neg.init({});


    adc1.init(
        {
            {AdcChannelIndex::VREF, AdcSampleCycles::T28_5}
        },{
            // {AdcChannelIndex::CH4, AdcSampleCycles::T28_5},
            {AdcChannelIndex::CH4, AdcSampleCycles::T28_5},
        }
    );

    adc1.set_injected_trigger(AdcInjectedTrigger::T3CC4);
    adc1.enable_auto_inject(false);
    // adc1.set_pga(AdcPga::X16);

    timer.set_trgo_source(TimerTrgoSource::OC4R);

    timer.oc(4).init({.install_en = DISEN});

    // timer.oc(4).cvr() = timer.arr() - 1; 
    // timer.oc(4).cvr() = int(timer.arr() * 0.1_r); 
    timer.oc(4).cvr() = int(1); 
    
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
        .fl = 150,
        .fh = 400,
        .fs = ISR_FREQ
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
    
    dsp::DeltaPdController pi_ctrl{{
        .kp = 0.8_r,
        .kd = 0.00_r,

        .out_min = 0.7_r,
        .out_max = 0.97_r,

        .fs = ISR_FREQ
    }};

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

    auto & watch_gpio = portA[3];
    watch_gpio.outpp();

    
    // while(true){
    //     DEBUG_PRINTLN('/');
    //     static bool i = 0;
    //     i = !i;
    //     // clock::delay(2us);
    //     // __nopn(60);
    //     // clock::delay(1ms);
    //     watch_gpio.write(i);
    // }

    adc1.attach(AdcIT::JEOC, {0,0}, [&](){
        watch_gpio.toggle();
        volt = adc1.inj(1).get_voltage();
        const auto curr_raw = volt_2_current(volt);

        lpf.update(curr_raw);
        lpf_mid.update(curr_raw);
        // curr = lpf.get();
        curr = curr_raw;
        watch_gpio.toggle();
        // bpf.update(curr_raw);

        bpf.update(curr_raw);
        curr_mid = lpf_mid.get();

        ect.update(bool(bpf.get() > 0));

        const auto pos = ect.count() * 0.01_r;
        td.update(pos);
        const auto spd = td.get()[1];

        // static constexpr auto kp = 267.0_r;
        // static constexpr auto kd = 0.0_r;
        // const auto spd_cmd = kp * (pos_targ - pos) + kd * (spd_targ - spd);
        pi_ctrl.update(spd_targ, spd);
        // pwm_pos = pi_ctrl.get();
        // pwm_pos = 0.87_r * abs(sinpu(time()));
        // pwm_pos = 0.7_r + 0.17_r * abs(sinpu(time()));
        // pwm_pos = 0.13_r + 0.817_r * abs(sinpu(time()));
        pwm_pos.set_duty(0_r);
        // pwm_neg = LERP(0.32_r, 0.32_r, sin(time()) * 0.5_r + 0.5_r);
        // pwm_neg = LERP(0.32_r, 0.32_r, sin(time()) * 0.5_r + 0.5_r);
        pwm_neg.set_duty(0.9_r);



        uart.tick();
    });


    drivers::AT8222 motdrv{
        pwm_pos, pwm_neg, hal::NullGpio
    };

    motdrv.init();

    


    while(true){
        // const auto duty = sin(4 * time()) * 0.2_r + 0.7_r;
        // motdrv = duty;
        // pwm_neg = 0.5_r;
        // pwm_pos = ABS(duty);

        // pwm_pos = 0.9_r + 0.1_r * sin(5 * time());
        // spd_targ = 7.0_r + 3 * sin(5 * time());
        // spd_targ = 8.0_r + 1.0_r * ((sin(2.0_r * time())) > 0 ? 1 : -1);
        // spd_targ = 8.0_r + 1.0_r * sinpu(2.0_r * time());
        const auto t = clock::time();

        #define TEST_MODE 1

        #if TEST_MODE == 0
        spd_targ = 12;
        pos_targ = 10.0_r * t + 2*frac(t);
        #elif TEST_MODE == 1
        spd_targ = 7.0_r + 1.0_r * sinpu(1.3_r * t);
        pos_targ = 7.0_r * t + real_t(-1.0/6) * cospu(1.3_r * t);
        #endif
        // spd_targ = 9.0_r + 1.0_r * ((sin(1.0_r * time())) > 0 ? 1 : ;
        // spd_targ = 9.0_r + 1.0_r * -1;
        // spd_targ = 16.57_r;
        // trackin_sig = sign(sin(t * 3));
        // trackin_sig = real_t(int(sin(t * 3) * 32)) / 32;
        // trackin_sig = real_t(int(0.2_r * sin(t * 3) * 32)) / 32;
        // trackin_sig = real_t(int(0.2_r * t * 32)) / 32;
        // trackin_sig = 1/(1 + exp(4 * tpzpu(3 * t)));
        trackin_sig = 10 * CLAMP2(sinpu(7 * t), 0.5_r);
        // trackin_sig = tpzpu(t);
        
        // DEBUG_PRINTLN_IDLE(pos_targ, spd_targ, bpf.get(), volt, pi_ctrl.get(), bpf.get(), , exe_micros);
        // DEBUG_PRINTLN_IDLE(td.get(), lpf.get() * 90 ,volt,spd_targ, exe_micros);
        DEBUG_PRINTLN_IDLE(curr * 1000,curr_mid * 1000, volt, curr > curr_mid);
        // DEBUG_PRINTLN_IDLE(trackin_sig, td.get());
        // DEBUG_PRINTLN(bool(pwm_pos.io()), bool(pwm_neg.io()));
        
    }
}