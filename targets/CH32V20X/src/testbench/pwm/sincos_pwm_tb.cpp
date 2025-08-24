#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/math/realmath.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "hal/analog/adc/adcs/adc1.hpp"

using namespace ymd;
using namespace ymd::hal;

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
    auto & timer = timer1;
    #elif TIM_INDEX == 2
    auto & timer = timer2;
    #elif TIM_INDEX == 3
    auto & timer = timer3;
    #elif TIM_INDEX == 4
    auto & timer = timer4;
    #endif

    auto & pwm_ap = timer.oc<1>();
    auto & pwm_an = timer.oc<2>();
    auto & pwm_bp = timer.oc<3>();
    auto & pwm_bn = timer.oc<4>();


    timer.init({FREQ, TimerCountMode::CenterAlignedDualTrig});
    timer.enable_arr_sync();

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
    timer.set_trgo_source(TimerTrgoSource::Update);
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

    const TimerOcPwmConfig pwm_noinv_cfg = {
        .cvr_sync_en = EN,
        .valid_level = HIGH
    };

    const TimerOcPwmConfig pwm_inv_cfg = {
        .cvr_sync_en = EN,
        .valid_level = LOW,
    };
    
    pwm_ap.init(pwm_noinv_cfg);
    pwm_an.init(pwm_noinv_cfg);
    pwm_bp.init(pwm_inv_cfg);
    pwm_bn.init(pwm_inv_cfg);

    TimerOcPair pwm_a = {pwm_ap, pwm_an};
    TimerOcPair pwm_b = {pwm_bp, pwm_bn};

    pwm_b.inverse(EN);

    adc1.init(
        {
            {AdcChannelNth::VREF, AdcSampleCycles::T28_5}
        },{
            {AdcChannelNth::CH5, AdcSampleCycles::T28_5},
        }, {}
    );

    #if TIM_INDEX == 1
    #if TIM1_USE_CC4
    adc1.set_injected_trigger(AdcInjectedTrigger::T1CC4);
    #else
    adc1.set_injected_trigger(AdcInjectedTrigger::T1TRGO);
    #endif
    #elif TIM_INDEX == 2
    adc1.set_injected_trigger(AdcInjectedTrigger::T2TRGO);
    #elif TIM_INDEX == 3
    adc1.set_injected_trigger(AdcInjectedTrigger::T3CC4);
    #elif TIM_INDEX == 4
    adc1.set_injected_trigger(AdcInjectedTrigger::T4TRGO);
    #endif

    adc1.enable_auto_inject(DISEN);

    auto & inj = adc1.inj<1>();

    auto & trig_gpio = portC[13];
    trig_gpio.outpp();

    adc1.attach(AdcIT::JEOC, {0,0}, [&]{
        trig_gpio.toggle();
        // DEBUG_PRINTLN_IDLE(millis());
    });
    

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