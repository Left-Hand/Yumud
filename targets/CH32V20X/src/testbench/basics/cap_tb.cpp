#include "src/testbench/tb.h"
#include "core/clock/time.hpp"
#include "core/clock/clock.hpp"
#include "core/math/realmath.hpp"
#include "core/stream/ostream.hpp"
#include "core/utils/default.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "core/sdk.hpp"


#define PWM_TB_GPIO
// #define PWM_TB_TIMEROC
// #define PWM_TB_CO


using namespace ymd;


//参考资料
// https://blog.csdn.net/weixin_49337111/article/details/135677928

//TIM输入捕获配置
[[maybe_unused]] void Capture_Wave_Init(void)
{
    TIM_ICInitTypeDef      TIM_ICInitStructure;    
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    hal::PB<0>().set_mode(hal::GpioMode::OutAfPP);
    
    
    TIM_TimeBaseInitStructure.TIM_Period = 65535;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    hal::NvicPriority{0, 1}.with_irqn(TIM3_IRQn).enable(EN);
    
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;


    //系数过大会导致锯齿严重 3比较合适
    // TIM_ICInitStructure.TIM_ICFilter = 0xF;
    // TIM_ICInitStructure.TIM_ICFilter = 0x1;
    TIM_ICInitStructure.TIM_ICFilter = 0x3;
    
    TIM_ICInit(TIM3, &TIM_ICInitStructure);
    
    TIM_Cmd(TIM3, ENABLE);
    
    TIM_ITConfig(TIM3, TIM_IT_CC3, ENABLE);
}
void cap_main(){
    auto & DBG_UART = hal::usart2;
    DBG_UART.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        // .baudrate = hal::NearestFreq(DEBUG_UART_BAUD),
        // .baudrate = hal::NearestFreq(6000000),
        .baudrate = hal::NearestFreq(576000),
        .tx_strategy = CommStrategy::Blocking,
    });
    
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);
    // DEBUGGER.force_sync(EN);

    auto & timer = hal::timer1;
    timer.init({
        .remap = hal::TIM1_REMAP_A8_A9_A10_A11__B13_B14_B15,
        // .count_freq = hal::NearestFreq(36000),
        .count_freq = hal::NearestFreq(20000),
        .count_mode = hal::TimerCountMode::Up
    }).unwrap()
    .alter_to_pins({
        hal::TimerChannelSelection::CH1,
        hal::TimerChannelSelection::CH2,
        hal::TimerChannelSelection::CH3,
    }).unwrap();
    timer.enable_arr_sync(EN);
    timer.bdtr().init({100ns});
    // timer.enable_arr_sync(DISEN);
    timer.start();
    
    
    auto & oc1 = timer.oc<1>();
    oc1.init(Default);
    auto & oc2 = timer.oc<2>();
    oc2.init(Default);


    Capture_Wave_Init();

    volatile bool is_up_edge = 1;
    volatile uint32_t high_val = 0;
    volatile uint32_t low_val = 0;

    auto watch_pin_ = hal::PA<4>();
    watch_pin_.outpp();

    hal::timer3.set_event_handler([&](const hal::TimerEvent & event){
        switch(event){
            case hal::TimerEvent::CC3:{

                if(is_up_edge == 1){
                    high_val = TIM_GetCapture3(TIM3);
                    TIM_OC3PolarityConfig(TIM3, TIM_ICPolarity_Falling);    //下降沿捕获    
                    TIM_SetCounter(TIM3, 0);
                    watch_pin_.set_low();
                    is_up_edge = false;
                }else{
                    low_val = TIM_GetCapture3(TIM3);
                    TIM_OC3PolarityConfig(TIM3, TIM_ICPolarity_Rising);    //上升沿捕获
                    watch_pin_.set_high();
                    is_up_edge = true;
                }
                break;
            }
            default: break;
        }
    });


    while(true){
        const auto now_secs = clock::seconds();

        const auto dutycycle = 0.4_iq16 * iq16(math::sinpu(4 * now_secs)) + 0.5_iq16;
        oc1.set_dutycycle(dutycycle);
        oc2.set_dutycycle(dutycycle);

        DEBUG_PRINTLN(
            // now_secs, 
            // oc1.cvr(), 
            // oc1.arr(), 
            // trig_cnt_, 
            // TIM3->CH1CVR,
            // TIM3->CH2CVR,
            // TIM3->CH3CVR,
            // cap_value_

            // is_up_edge,
            dutycycle,

            iq16(low_val)/high_val,
            low_val,
            high_val,
            TIM1->CH1CVR,
            TIM1->ATRLR

            // TIM3->CH4CVR
        );
    }

}