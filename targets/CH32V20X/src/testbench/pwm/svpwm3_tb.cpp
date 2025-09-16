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


// 适用于步进电机驱动单电阻采样方案的正交pwm输出
// 其中A相与B相的采样点错开
// 可以配置不同的串口输出和定时器（在下方以宏给出）

// 外设：
// UART:576000波特率输出，用于观察信号 
// TIM:CH1和CH2构成A相驱动芯片的两个输入端 CH3和CH4构成B相驱动芯片的两个输入端

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
    timer.init(
        {.freq = 20000, .mode = hal::TimerCountMode::CenterAlignedUpTrig},
        EN
    );

    timer.oc<4>().init({});
    timer.oc<4>().enable_output(EN);
    timer.oc<4>().cvr() = timer.arr() - 1;

    pwm_u.init(pwm_noinv_cfg);
    pwm_v.init(pwm_noinv_cfg);
    pwm_w.init(pwm_noinv_cfg);


    hal::adc1.init(
        {
            {hal::AdcChannelNth::VREF, hal::AdcSampleCycles::T28_5}
        },{
            {hal::AdcChannelNth::CH5, hal::AdcSampleCycles::T28_5},
        }, {}
    );

    #if TIM_INDEX == 1
    #if TIM1_USE_CC4
    hal::adc1.set_injected_trigger(AdcInjectedTrigger::T1CC4);
    #else
    hal::adc1.set_injected_trigger(hal::AdcInjectedTrigger::T1TRGO);
    #endif
    #elif TIM_INDEX == 2
    hal::adc1.set_injected_trigger(AdcInjectedTrigger::T2TRGO);
    #elif TIM_INDEX == 3
    hal::adc1.set_injected_trigger(AdcInjectedTrigger::T3CC4);
    #elif TIM_INDEX == 4
    hal::adc1.set_injected_trigger(AdcInjectedTrigger::T4TRGO);
    #endif

    hal::adc1.enable_auto_inject(DISEN);

    auto & inj = hal::adc1.inj<1>();

    auto trig_gpio = hal::PC<13>();
    trig_gpio.outpp();

    
    hal::adc1.attach(hal::AdcIT::JEOC, {0,0}, [&]{
        trig_gpio.toggle();
        // DEBUG_PRINTLN_IDLE(millis());
    }, EN);
    

    // timer.attach(TimerIT::CC4,{0,0}, [&]{
    //     trig_gpio = !trig_gpio;
    // });

    while(true){
        
        const auto t = clock::time() * real_t(5 * TAU);
        const auto [st,ct] = sincos(t);
        const auto [u, v, w] = digipw::SVM({st * 0.5_r, ct * 0.5_r});

        pwm_u.set_dutycycle(u);
        pwm_v.set_dutycycle(v);
        pwm_w.set_dutycycle(w);

        DEBUG_PRINTLN_IDLE(st, ct, inj.get_voltage());
    }
}