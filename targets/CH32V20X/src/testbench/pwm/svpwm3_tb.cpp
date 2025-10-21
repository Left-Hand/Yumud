#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/math/realmath.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "hal/analog/adc/adcs/adc1.hpp"

#include "digipw/SVPWM/svpwm3.hpp"

using namespace ymd;


static constexpr size_t CHOP_FREQ = 40_KHz;
// #define CHOP_FREQ 200

// #define UART uart1
#define UART hal::uart2

#define TIM_INDEX 1
// #define TIM_INDEX 2
// #define TIM_INDEX 3
// #define TIM_INDEX 4

#define TIM1_USE_CC4 0
// #define TIM1_USE_CC4 1

void svpwm3_main(){
    UART.init({576000});
    DEBUGGER.retarget(&UART);

    
    #if TIM_INDEX == 1
    auto & timer = hal::timer1;
    #elif TIM_INDEX == 2
    auto & timer = hal::timer2;
    #elif TIM_INDEX == 3
    auto & timer = hal::timer3;
    #elif TIM_INDEX == 4
    auto & timer = hal::timer4;
    #endif

    auto & pwm_u = timer.oc<1>();
    auto & pwm_v = timer.oc<2>();
    auto & pwm_w = timer.oc<3>();


    timer.init({CHOP_FREQ, hal::TimerCountMode::CenterAlignedDualTrig}, EN);
    timer.enable_arr_sync(EN);

    #if TIM_INDEX == 1
    #if TIM1_USE_CC4
    auto & trig_oc = timer.oc(4);
    trig_oc.init(TimerOcMode::UpValid, false);
    trig_oc.enable_output(true);
    trig_oc.set_idle_state(false);
    // trig_oc.cvr() = timer.arr() / 2;
    trig_oc.cvr() = timer.arr() - 10;
    // trig_oc.cvr() = 10;
    #else
    //重要!!!!
    timer.set_trgo_source(hal::TimerTrgoSource::Update);
    #endif
    #elif TIM_INDEX == 2
    //重要!!!!
    timer.set_trgo_source(TimerTrgoSource::Update);
    #elif TIM_INDEX == 3
    auto & trig_oc = timer.oc(4);
    trig_oc.init(TimerOcMode::UpValid, false);
    trig_oc.enable_output(true);
    trig_oc.set_idle_state(false);
    // trig_oc.cvr() = timer.arr() / 2;
    trig_oc.cvr() = timer.arr() - 1;
    #elif TIM_INDEX == 4
    //重要!!!!
    timer.set_trgo_source(TimerTrgoSource::Update);
    #endif

    const auto pwm_noinv_cfg = hal::TimerOcPwmConfig{
        .cvr_sync_en = EN,
        .valid_level = HIGH
    };

    // timer.init(CHOP_FREQ, TimerCountMode::CenterAlignedUpTrig);
    timer.init({
            .freq = 20000, 
            .count_mode = hal::TimerCountMode::CenterAlignedUpTrig
        },
        EN
    );

    timer.oc<4>().init({});
    timer.oc<4>().enable_output(EN);
    timer.oc<4>().cvr() = timer.arr() - 1;

    pwm_u.init(pwm_noinv_cfg);
    pwm_v.init(pwm_noinv_cfg);
    pwm_w.init(pwm_noinv_cfg);

    auto & adc = hal::adc1;

    adc.init(
        {
            {hal::AdcChannelSelection::VREF, hal::AdcSampleCycles::T28_5}
        },{
            {hal::AdcChannelSelection::CH5, hal::AdcSampleCycles::T28_5},
        }, {}
    );

    #if TIM_INDEX == 1
    #if TIM1_USE_CC4
    adc.set_injected_trigger(AdcInjectedTrigger::T1CC4);
    #else
    adc.set_injected_trigger(hal::AdcInjectedTrigger::T1TRGO);
    #endif
    #elif TIM_INDEX == 2
    adc.set_injected_trigger(AdcInjectedTrigger::T2TRGO);
    #elif TIM_INDEX == 3
    adc.set_injected_trigger(AdcInjectedTrigger::T3CC4);
    #elif TIM_INDEX == 4
    adc.set_injected_trigger(AdcInjectedTrigger::T4TRGO);
    #endif


    adc.enable_auto_inject(DISEN);

    auto & inj = adc.inj<1>();

    auto trig_gpio = hal::PC<13>();
    trig_gpio.outpp();

    adc.register_nvic({0,0}, EN);
    adc.enable_interrupt<hal::AdcIT::JEOC>(EN);
    adc.set_event_callback(
        [&](const hal::AdcEvent ev){
            switch(ev){
            case hal::AdcEvent::EndOfInjectedConversion:{
                trig_gpio.toggle();
                break;}
            default: break;
            }
        }
    );
    


    while(true){
        
        const auto ctime = clock::time() * real_t(5 * TAU);
        const auto [st,ct] = sincos(ctime);
        const auto [u, v, w] = digipw::SVM({st * 0.5_r, ct * 0.5_r});

        pwm_u.set_dutycycle(u);
        pwm_v.set_dutycycle(v);
        pwm_w.set_dutycycle(w);

        DEBUG_PRINTLN_IDLE(st, ct, inj.get_voltage());
    }
}