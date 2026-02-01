#include "src/testbench/tb.h"
#include "core/clock/time.hpp"
#include "core/clock/clock.hpp"
#include "core/math/realmath.hpp"
#include "core/stream/ostream.hpp"
#include "core/utils/default.hpp"

#include "hal/dma/dma.hpp"
#include "hal/bus/uart/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "core/sdk.hpp"

#include "drivers/Modem/dshot/dshot.hpp"

using namespace ymd;

using namespace ymd::drivers;


void dshot_main(){

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

    auto & timer = hal::timer3;

    timer.init({
        .remap = hal::TIM3_REMAP_A6_A7_B0_B1,
        .count_freq = hal::NearestFreq(20_KHz),
        // .count_freq = hal::NearestFreq(600_KHz),
        .count_mode = hal::TimerCountMode::Up
    }).unwrap()
        .alter_to_pins({
            // hal::TimerChannelSelection::CH1,
            // hal::TimerChannelSelection::CH2,
            hal::TimerChannelSelection::CH3,
            hal::TimerChannelSelection::CH4,
        })
        .unwrap();
    // TIM_DMACmd(TIM3, TIM_DMA_CC1, ENABLE);
    // TIM_DMACmd(TIM3, TIM_DMA_CC2, ENABLE);
    // TIM_DMACmd(TIM3, TIM_DMA_CC3, ENABLE);
    TIM_DMACmd(TIM3, TIM_DMA_CC4, ENABLE);
    timer.start();
    // auto & oc3 = timer.oc<3>();
    auto & oc4 = timer.oc<4>();

    // DShotChannel ch1{oc3};
    DShotChannel ch2{oc4};

    // ch1.init();
    ch2.init();

    while(true){
        const auto now_secs = clock::seconds();

        const auto dutycycle = 0.4_iq16 * iq16(math::sinpu(4 * now_secs)) + 0.5_iq16;
        const auto dutycycle_int = static_cast<uint16_t>(600 * dutycycle);
        // ch1.set_content(dutycycle_int);
        ch2.set_content(dutycycle_int);

        clock::delay(1ms);
        // auto & dma = oc3.dma().unwrap();
        // DEBUG_PRINTLN(
            // std::hex, reinterpret_cast<uint32_t>(dma.inst_),
            // DMA1_Channel2_BASE
            // dma.pending_count
        // );
        // while(oc3.dma().unwrap().pending_count() != 0){
        //     __nop;
        // }
    }
}