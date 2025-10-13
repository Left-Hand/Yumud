#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/math/realmath.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "hal/analog/adc/adcs/adc1.hpp"

using namespace ymd;


// 适用于步进电机驱动单电阻采样方案的正交pwm输出
// 其中A相与B相的采样点错开
// 可以配置不同的串口输出和定时器（在下方以宏给出）

// 外设：
// UART:576000波特率输出，用于观察信号 
// TIM:CH1和CH2构成A相驱动芯片的两个输入端 CH3和CH4构成B相驱动芯片的两个输入端

static constexpr size_t FREQ = 2_KHz;
// #define FREQ 200

// #define UART uart1
#define UART hal::uart2

#define TIM_INDEX 1
// #define TIM_INDEX 2
// #define TIM_INDEX 3
// #define TIM_INDEX 4

#define TIM1_USE_CC4 0
// #define TIM1_USE_CC4 1

void sincos_pwm_main(){
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

    auto & pwm_ap = timer.oc<1>();
    auto & pwm_an = timer.oc<2>();
    auto & pwm_bp = timer.oc<3>();
    auto & pwm_bn = timer.oc<4>();


    timer.init({FREQ, hal::TimerCountMode::CenterAlignedDualTrig}, EN);
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

    const hal::TimerOcPwmConfig pwm_noinv_cfg = {
        .cvr_sync_en = EN,
        .valid_level = HIGH
    };

    const hal::TimerOcPwmConfig pwm_inv_cfg = {
        .cvr_sync_en = EN,
        .valid_level = LOW,
    };
    
    pwm_ap.init(pwm_noinv_cfg);
    pwm_an.init(pwm_noinv_cfg);
    pwm_bp.init(pwm_inv_cfg);
    pwm_bn.init(pwm_inv_cfg);

    auto pwm_a = hal::TimerOcPair{pwm_ap, pwm_an};
    auto pwm_b = hal::TimerOcPair{pwm_bp, pwm_bn};

    pwm_b.inverse(EN);

    auto & adc = hal::adc1;

    adc.init(
        {
            {hal::AdcChannelNth::VREF, hal::AdcSampleCycles::T28_5}
        },{
            {hal::AdcChannelNth::CH5, hal::AdcSampleCycles::T28_5},
        }, {}
    );

    #if TIM_INDEX == 1
    #if TIM1_USE_CC4
    adc.set_injected_trigger(hal::AdcInjectedTrigger::T1CC4);
    #else
    adc.set_injected_trigger(hal::AdcInjectedTrigger::T1TRGO);
    #endif
    #elif TIM_INDEX == 2
    adc.set_injected_trigger(hal::AdcInjectedTrigger::T2TRGO);
    #elif TIM_INDEX == 3
    adc.set_injected_trigger(hal::AdcInjectedTrigger::T3CC4);
    #elif TIM_INDEX == 4
    adc.set_injected_trigger(hal::AdcInjectedTrigger::T4TRGO);
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


    // timer.attach(TimerIT::CC4,{0,0}, [&]{
    //     trig_gpio = !trig_gpio;
    // });

    while(true){
        
        const auto t = clock::time() * real_t(3 * TAU);
        const auto [st, ct] = sincospu(t);
        
        pwm_a.set_dutycycle(st);
        pwm_b.set_dutycycle(ct);

        DEBUG_PRINTLN_IDLE(st, ct, inj.get_voltage());
    }
}