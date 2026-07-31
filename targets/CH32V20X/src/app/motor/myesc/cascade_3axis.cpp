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


#ifndef CH32V30X
#define CH32V30X
#endif


#ifdef CH32V30X


void TIM_MultiAxisSync_Init(void)
{

    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM1, DISABLE);
    GPIO_PinRemapConfig(GPIO_Remap_TIM8, DISABLE);
    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM10, DISABLE);

    static constexpr size_t PSC_VALUE = 480 - 1;
    static constexpr size_t ARR_VALUE = 1000 - 1;



    TIM_OCInitTypeDef TIM_OCInitStructure = {
        .TIM_OCMode       = TIM_OCMode_PWM1,
        .TIM_OutputState  = TIM_OutputState_Enable,
        .TIM_OutputNState = TIM_OutputState_Enable,   // 若有互补输出
        .TIM_Pulse        = 200,                      // 50% 占空比（可根据需要修改）
        .TIM_OCPolarity   = TIM_OCPolarity_High,
        .TIM_OCNPolarity  = TIM_OCNPolarity_Low,
        .TIM_OCIdleState  = TIM_OCIdleState_Reset,
        .TIM_OCNIdleState = TIM_OCNIdleState_Reset,
    };

    // 1. 时钟使能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    //tim1 ch1->PA8
    {
        GPIO_InitTypeDef GPIO_InitStructure;

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init( GPIOA, &GPIO_InitStructure );
    }
    //tim8 ch1->PC6
    {
        GPIO_InitTypeDef GPIO_InitStructure;

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init( GPIOC, &GPIO_InitStructure );
    }
    //tim10 ch1->PB8
    {
        GPIO_InitTypeDef GPIO_InitStructure;

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init( GPIOB, &GPIO_InitStructure );
    }

    // ============ 配置 TIM2（同步主触发器） ============
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
    TIM_TimeBaseInitStruct.TIM_Period = 1;          // ARR=1，产生快速更新事件
    TIM_TimeBaseInitStruct.TIM_Prescaler = 0;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);

    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update); // TRGO 输出更新事件


    // ============ 配置 TIM1（相位 0°） ============
    TIM_CounterModeConfig(TIM1, TIM_CounterMode_CenterAligned1);
    TIM_SetAutoreload(TIM1, ARR_VALUE);
    TIM_PrescalerConfig(TIM1, PSC_VALUE, TIM_PSCReloadMode_Immediate);
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_SetCounter(TIM1, 0);   // 初始值0

    TIM_SelectInputTrigger(TIM1, TIM_TS_ITR1);
    TIM_SelectSlaveMode(TIM1, TIM_SlaveMode_Trigger);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    // ============ 配置 TIM8（相位 120°） ============
    TIM_CounterModeConfig(TIM8, TIM_CounterMode_CenterAligned1);
    TIM_SetAutoreload(TIM8, ARR_VALUE);
    TIM_PrescalerConfig(TIM8, PSC_VALUE, TIM_PSCReloadMode_Immediate);
    TIM_OC1Init(TIM8, &TIM_OCInitStructure);
    TIM_SetCounter(TIM8, 333);   // 333/1000*360° = 120°

    TIM_SelectInputTrigger(TIM8, TIM_TS_ITR1);
    TIM_SelectSlaveMode(TIM8, TIM_SlaveMode_Trigger);
    TIM_CtrlPWMOutputs(TIM8, ENABLE);

    // ============ 配置 TIM10（相位 240°） ============
    TIM_CounterModeConfig(TIM10, TIM_CounterMode_CenterAligned1);
    TIM_SetAutoreload(TIM10, ARR_VALUE);
    TIM_PrescalerConfig(TIM10, PSC_VALUE, TIM_PSCReloadMode_Immediate);
    TIM_OC1Init(TIM10, &TIM_OCInitStructure);
    TIM_SetCounter(TIM10, 666);   // 666/1000*360° = 240°

    TIM_SelectInputTrigger(TIM10, TIM_TS_ITR1);
    TIM_SelectSlaveMode(TIM10, TIM_SlaveMode_Trigger);
    TIM_CtrlPWMOutputs(TIM10, ENABLE);

    // ============ 启动定时器 ============
    TIM_Cmd(TIM1, ENABLE);
    TIM_Cmd(TIM8, ENABLE);
    TIM_Cmd(TIM10, ENABLE);

    TIM_Cmd(TIM2, ENABLE);
    TIM_GenerateEvent(TIM2, TIM_EventSource_Trigger);
    TIM_Cmd(TIM2, DISABLE);
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

    // hal::PA<8>().afpp();
    // hal::PC<6>().afpp();
    // hal::PB<8>().afpp();

    TIM_MultiAxisSync_Init();



    // const auto oc_mode = hal::TimerOcMode::ActiveAboveCvr;
    // setup_timeroc(TIM1, hal::TimerChannelSelection::CH1, oc_mode);
    // setup_timeroc(TIM8, hal::TimerChannelSelection::CH1, oc_mode);
    // setup_timeroc(TIM10, hal::TimerChannelSelection::CH1, oc_mode);


    TIM1->CH1CVR = 200;
    TIM8->CH1CVR = 300;
    TIM10->CH1CVR = 400;

    while(true){
        DEBUG_PRINTLN(uint16_t(TIM1->CNT), uint16_t(TIM8->CNT), uint16_t(TIM10->CNT));
        poll_led_blink();
        // clock::delay(2ms);
    }
}
#endif