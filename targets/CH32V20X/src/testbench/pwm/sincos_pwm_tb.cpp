#include "src/testbench/tb.h"

#include "sys/debug/debug.hpp"
#include "sys/clock/time.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/adc/adcs/adc1.hpp"

// 适用于步进电机驱动单电阻采样方案的正交pwm输出
// 其中A相与B相的采样点错开
// 可以配置不同的串口输出和定时器（在下方以宏给出）

// 外设：
// UART:576000波特率输出，用于观察信号 
// TIM:CH1和CH2构成A相驱动芯片的两个输入端 CH3和CH4构成B相驱动芯片的两个输入端

#define UART_INDEX 1
// #define UART_INDEX 2

// #define TIM_INDEX 1
#define TIM_INDEX 2
// #define TIM_INDEX 3
// #define TIM_INDEX 4

// #define TIM1_USE_CC4 0
#define TIM1_USE_CC4 1

class TimerOCPair:public PwmChannelIntf{
protected:
    TimerOC & oc_;
    TimerOC & ocn_;
    bool inversed = false;
public:
    TimerOCPair(AdvancedTimer & timer, const size_t idx, const size_t idx2):
        oc_(timer.oc(idx)),
        ocn_(timer.oc(idx2)){;}

    TimerOCPair(TimerOC & oc, TimerOC & ocn):
        oc_(oc), ocn_(ocn){;}

    TimerOCPair & operator = (const real_t value) override{
        const bool is_minus = signbit(value);
        const auto abs_value = inversed ? (1 - ABS(value)) : ABS(value);
        const auto zero_value = real_t(inversed);

        if(is_minus){
            oc_ = zero_value;
            ocn_ = abs_value;
        }else{
            oc_ = abs_value;
            ocn_ = zero_value;
        }
        
        return *this;
    }

    void inverse(const bool en){
        inversed = en;
    }
};
    

void sincos_pwm_main(){
    #if UART_INDEX == 1
    auto & uart = uart1;
    #elif UART_INDEX == 2
    auto & uart = uart2;
    #endif
    uart.init(576000);
    DEBUGGER.retarget(&uart);

    
    #if TIM_INDEX == 1
    auto & timer = timer1;
    #elif TIM_INDEX == 2
    auto & timer = timer2;
    #elif TIM_INDEX == 3
    auto & timer = timer3;
    #elif TIM_INDEX == 4
    auto & timer = timer4;
    #endif

    auto & pwm_ap = timer.oc(1);
    auto & pwm_an = timer.oc(2);
    auto & pwm_bp = timer.oc(3);
    auto & pwm_bn = timer.oc(4);


    timer.init(40_KHz, TimerMode::CenterAlignedDualTrig);
    timer.enableArrSync();

    #if TIM_INDEX == 1
    #if TIM1_USE_CC4
    auto & trig_oc = timer.oc(4);
    trig_oc.init(TimerUtils::OcMode::UpValid, false)
    .setOutputState(true)
    .setIdleState(false)
    ;
    // trig_oc.cvr() = timer.arr() / 2;
    trig_oc.cvr() = timer.arr() - 10;
    // trig_oc.cvr() = 10;
    #else
    //重要!!!!
    timer.setTrgoSource(TimerUtils::TrgoSource::Update);
    #endif
    #elif TIM_INDEX == 2
    //重要!!!!
    timer.setTrgoSource(TimerTrgoSource::Update);
    #elif TIM_INDEX == 3
    auto & trig_oc = timer.oc(4);
    trig_oc.init(TimerUtils::OcMode::UpValid, false)
    .setOutputState(true)
    .setIdleState(false)
    ;
    // trig_oc.cvr() = timer.arr() / 2;
    trig_oc.cvr() = timer.arr() - 1;
    #elif TIM_INDEX == 4
    //重要!!!!
    timer.setTrgoSource(TimerUtils::TrgoSource::Update);
    #endif

    pwm_ap.init();
    pwm_ap.sync();
    pwm_an.init();
    pwm_an.sync();

    pwm_bp.init();
    pwm_bp.sync();
    pwm_bn.init();
    pwm_bn.sync();

    pwm_bp.setPolarity(false);
    pwm_bn.setPolarity(false);

    TimerOCPair pwm_a = {pwm_ap, pwm_an};
    TimerOCPair pwm_b = {pwm_bp, pwm_bn};

    pwm_b.inverse(true);


    using AdcChannelIndex = AdcUtils::ChannelIndex;
    using AdcCycles = AdcUtils::SampleCycles;


    adc1.init(
        {
            {AdcChannelIndex::VREF, AdcCycles::T28_5}
        },{
            {AdcChannelIndex::CH5, AdcCycles::T28_5},
            // {AdcChannelIndex::CH4, AdcCycles::T28_5},
            // {AdcChannelIndex::CH1, AdcCycles::T28_5},
        }
    );

    #if TIM_INDEX == 1
    #if TIM1_USE_CC4
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T1CC4);
    #else
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T1TRGO);
    #endif
    #elif TIM_INDEX == 2
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T2TRGO);
    #elif TIM_INDEX == 3
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T3CC4);
    #elif TIM_INDEX == 4
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T4TRGO);
    #endif

    adc1.enableAutoInject(false);

    auto & inj = adc1.inj(1);

    auto & trig_gpio = portC[13];
    trig_gpio.outpp();

    adc1.bindCb(AdcUtils::IT::JEOC, [&]{
        trig_gpio = !trig_gpio;
    });
    
    adc1.enableIT(AdcUtils::IT::JEOC, {0,0});
    
    // timer.bindCb(TimerUtils::IT::CC4, [&]{
    //     trig_gpio = !trig_gpio;
    // });

    // timer.enableIt(TimerUtils::IT::CC4,{0,0});

    while(true){
        
        const auto t = time() * real_t(50 * TAU);
        const auto st = sin<30>(t);
        const auto ct = cos<30>(t);
        
        pwm_a = st;
        pwm_b = ct;

        DEBUG_PRINTLN_IDLE(st, ct, real_t(inj));
    }
}