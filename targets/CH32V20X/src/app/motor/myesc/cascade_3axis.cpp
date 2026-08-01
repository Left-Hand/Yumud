#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/utils/default.hpp"
#include "core/async/timer.hpp"

#include "hal/timer/hw_singleton.hpp"
#include "hal/conn/uart/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/analog/adc/hw_singleton.hpp"
#include "hal/conn/uart/hw_singleton.hpp"
#include "hal/conn/spi/hw_singleton.hpp"
#include "hal/dma/dma.hpp"


#include "drivers/encoder/magnetic/MT6825/mt6825.hpp"
#include "drivers/encoder/magnetic/VCE2755/vce2755.hpp"
#include "drivers/gatedrv/DRV832X/DRV8323h.hpp"

#include "core/sdk.hpp"

using namespace ymd;



#ifdef CH32V30X


static constexpr size_t PSC_VALUE = 1 - 1;
static constexpr size_t ARR_VALUE = 4900 - 1;
static constexpr size_t ARR_VALUE_2_BY_3 = (ARR_VALUE * 2) / 3;

static constexpr bool PHASE_INVERT_EN = false;
// static constexpr bool PHASE_INVERT_EN = false;

#define INVERT_PHASE_IF_NEEDED \
{if constexpr(PHASE_INVERT_EN){\
    ch1_duty = ARR_VALUE - ch1_duty;\
    ch2_duty = ARR_VALUE - ch2_duty;\
    ch3_duty = ARR_VALUE - ch3_duty;\
}};\


static void set_timer1_duty(uint16_t ch1_duty, uint16_t ch2_duty, uint16_t ch3_duty){
    INVERT_PHASE_IF_NEEDED
    TIM1->CH1CVR = ch1_duty;
    TIM1->CH2CVR = ch2_duty;
    TIM1->CH3CVR = ch3_duty;
}

static void set_timer8_duty(uint16_t ch1_duty, uint16_t ch2_duty, uint16_t ch3_duty){
    INVERT_PHASE_IF_NEEDED
    TIM8->CH1CVR = ch1_duty;
    TIM8->CH2CVR = ch2_duty;
    TIM8->CH3CVR = ch3_duty;
}

static void set_timer10_duty(uint16_t ch1_duty, uint16_t ch2_duty, uint16_t ch3_duty){
    INVERT_PHASE_IF_NEEDED
    TIM10->CH1CVR = ch1_duty;
    TIM10->CH2CVR = ch2_duty;
    TIM10->CH3CVR = ch3_duty;
}


void init_pwmgen_timers_interleaved(void)
{

    // GPIO_PinRemapConfig(GPIO_PartialRemap_TIM1, DISABLE);
    // GPIO_PinRemapConfig(GPIO_Remap_TIM8, DISABLE);
    // GPIO_PinRemapConfig(GPIO_PartialRemap_TIM10, DISABLE);


    static constexpr TIM_OCInitTypeDef TIM_OCInitStructure = {
        .TIM_OCMode       = TIM_OCMode_PWM1,
        .TIM_OutputState  = TIM_OutputState_Enable,
        .TIM_OutputNState = TIM_OutputState_Enable,
        .TIM_Pulse        = 0,

        #if 1
        .TIM_OCPolarity   = TIM_OCPolarity_High,
        .TIM_OCNPolarity  = TIM_OCNPolarity_Low,
        #else
        .TIM_OCPolarity   = TIM_OCPolarity_High,
        .TIM_OCNPolarity  = TIM_OCNPolarity_Low,
        #endif

        .TIM_OCIdleState  = TIM_OCIdleState_Reset,
        .TIM_OCNIdleState = TIM_OCNIdleState_Reset,
    };

    // 1. 时钟使能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);


    // ============ 配置 TIM2（同步主触发器） ============
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
    TIM_TimeBaseInitStruct.TIM_Period = ARR_VALUE_2_BY_3;
    TIM_TimeBaseInitStruct.TIM_Prescaler = PSC_VALUE;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    TIM2->CH1CVR = ARR_VALUE / 10;

    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update); // TRGO 输出更新事件

    auto setup_pwmgen_timer = [](TIM_TypeDef * inst){
        static constexpr uint16_t INITIAL_COUNT_VALUE = 0;
        TIM_CounterModeConfig(inst, TIM_CounterMode_CenterAligned1);
        TIM_SetAutoreload(inst, ARR_VALUE);
        TIM_PrescalerConfig(inst, PSC_VALUE, TIM_PSCReloadMode_Immediate);
        TIM_OC1Init(inst, &TIM_OCInitStructure);
        TIM_OC2Init(inst, &TIM_OCInitStructure);
        TIM_OC3Init(inst, &TIM_OCInitStructure);
        TIM_SetCounter(inst, INITIAL_COUNT_VALUE);

        TIM_SelectInputTrigger(inst, TIM_TS_ITR1);
        TIM_SelectSlaveMode(inst, TIM_SlaveMode_Trigger);

        TIM_Cmd(inst, ENABLE);
    };


    hal::timer2.register_nvic<hal::TimerIT::Update>(hal::NvicPriorityCode::highest(), EN);
    hal::timer2.enable_interrupt<hal::TimerIT::Update>(EN);

    hal::timer2.set_event_callback([&](const hal::TimerEvent ev){
        switch(ev){
            case hal::TimerEvent::Update:{
                static uint8_t cnt = 0;

                switch(cnt){
                    case 0:
                        setup_pwmgen_timer(TIM1);
                        cnt = 1;
                        break;
                    case 1:
                        setup_pwmgen_timer(TIM8);
                        cnt = 2;
                        break;
                    case 2:
                        setup_pwmgen_timer(TIM10);
                        TIM_Cmd(TIM2, DISABLE);
                        cnt = 3;
                        break;
                }
                break;
            }
            default:
                break;
        };
    });

    
    TIM_Cmd(TIM2, ENABLE);

    clock::delay(10ms);

    hal::timer2.set_event_callback(nullptr);

    TIM_Cmd(TIM2, DISABLE);
}


static uint8_t calc_bottom_pwmgen_nth(){
    const uint16_t timer1_cnt = TIM1->CNT;
    const uint16_t timer8_cnt = TIM8->CNT;
    const uint16_t timer10_cnt = TIM10->CNT;

    uint8_t index = 0;
    uint16_t min_cnt = timer1_cnt;
    if(timer8_cnt < min_cnt){
        min_cnt = timer8_cnt;
        index = 1;
    }

    if(timer10_cnt < min_cnt){
        index = 2;
    }

    return index;
}

[[maybe_unused]] static void setup_timeroc(
    void * p_inst,
    const hal::TimerChannelSelection ch_sel,
    const hal::TimerOcMode oc_mode
){
    lld::timeroc_set_oc_mode(p_inst, ch_sel, oc_mode);
    lld::timeroc_enable_cvr_sync(p_inst, ch_sel, DISEN);
    lld::timeroc_set_valid_level(p_inst, ch_sel, HIGH);
    lld::timeroc_enable_output(p_inst, ch_sel, EN);
}

#define DBG_UART DEBUGGER_INST

void axis3_main(){
    //等待板载外围原件稳定（如SPI编码器）
    clock::delay(12ms);

    DBG_UART.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        // .baudrate = hal::NearestFreq(DEBUG_UART_BAUD),
        // .baudrate = hal::NearestFreq(6000000),
        .baudrate = hal::NearestFreq(576000),
        .rx_strategy = CommStrategy::Dma,
        .tx_strategy = CommStrategy::Blocking,
    });

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.build_config()
        .set_eps(5)
        .set_splitter(",")
        .no_brackets(EN)
        .no_fieldname(EN)
        .force_sync(EN)
        .finalize();

    clock::delay(2ms);

    auto led_blue_pin_ = hal::PC<13>();

    led_blue_pin_.outpp();

    [[maybe_unused]] auto poll_led_blink = [&]{
        const auto millis_u32 = uint32_t(clock::millis().count());
        led_blue_pin_ = BoolLevel::from((millis_u32 % 200u) > 100);
    };

    hal::PA<0>().afpp();
    hal::PA<8>().afpp();
    hal::PC<6>().afpp();
    hal::PB<8>().afpp();

    init_pwmgen_timers_interleaved();

    auto timming_watch_pin_ = hal::PA<12>();
    timming_watch_pin_.outpp();



    hal::adc1.init({
            {hal::AdcChannelSelection::VREF, hal::AdcSampleCycles::T28_5}
        },{

            #if 0
            {hal::AdcChannelSelection::CH1, hal::AdcSampleCycles::T1_5},
            // {hal::AdcChannelSelection::CH4, hal::AdcSampleCycles::T13_5},
            // {hal::AdcChannelSelection::CH5, hal::AdcSampleCycles::T13_5},  
            #else
            {hal::AdcChannelSelection::CH1, hal::AdcSampleCycles::T28_5},
            {hal::AdcChannelSelection::CH4, hal::AdcSampleCycles::T28_5},
            {hal::AdcChannelSelection::CH5, hal::AdcSampleCycles::T28_5},  
            #endif
            // {hal::AdcChannelSelection::TEMP, hal::AdcSampleCycles::T28_5},  

            // {hal::AdcChannelSelection::CH1, hal::AdcSampleCycles::T7_5},
            // {hal::AdcChannelSelection::CH4, hal::AdcSampleCycles::T7_5},
            // {hal::AdcChannelSelection::CH5, hal::AdcSampleCycles::T7_5},

        },
        {}
    );


    hal::adc1.set_injected_trigger(hal::AdcInjectedTrigger::T1CC4);
    hal::adc1.enable_auto_inject(DISEN);
    hal::adc1.register_nvic(hal::NvicPriorityCode::highest(),  EN);
    hal::adc1.enable_interrupt(hal::AdcIT::JEOC, EN);


    auto jeoc_isr = [&]{

        auto quick_delay = [&]{
            for(size_t i = 0; i < 30; i++) __nopn(1);
        };

        auto blink_watch = [&]{
            timming_watch_pin_.set_high();
            quick_delay();
            timming_watch_pin_.set_low();
            quick_delay();
        };

        const size_t blink_times = calc_bottom_pwmgen_nth() + 1;

        for(size_t i = 0; i < blink_times; i++){
            blink_watch();
        }
    };


    hal::adc1.set_event_callback(
        [&](const hal::AdcEvent ev){
            switch(ev){
            case hal::AdcEvent::EndOfInjectedConversion:{
                jeoc_isr();
                break;
            }
            default: break;
            }
        }
    );
    #if 1
    hal::timer1.oc<4>().init({
        .oc_mode = hal::TimerOcMode::ActiveAboveCvr,
        .cvr_sync_en = EN,
        .valid_level = HIGH,
        .out_en = EN
    });

    TIM1->CH4CVR = ARR_VALUE - 8;
    #endif

    set_timer1_duty(200, 300, 400);
    set_timer8_duty(300, 300, 400);
    set_timer10_duty(400, 300, 400);

    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM8, ENABLE);
    TIM_CtrlPWMOutputs(TIM10, ENABLE);


    while(true){
        DEBUG_PRINTLN(uint16_t(TIM1->CNT), uint16_t(TIM8->CNT), uint16_t(TIM10->CNT));
        poll_led_blink();
        // clock::delay(2ms);
    }
}
#endif