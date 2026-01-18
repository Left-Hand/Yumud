#include "src/testbench/tb.h"
#include "core/clock/time.hpp"
#include "core/clock/clock.hpp"
#include "core/math/realmath.hpp"
#include "core/stream/ostream.hpp"
#include "core/utils/default.hpp"

#include "hal/dma/dma.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "core/sdk.hpp"

#include "drivers/Modem/dshot/dshot.hpp"

using namespace ymd;

using namespace ymd::drivers;

static constexpr size_t N = 40;
// static std::array<uint16_t, N> data;


[[maybe_unused]] static constexpr uint16_t dshot_crc(uint16_t data_in){
	uint16_t speed_data;
	speed_data = data_in << 5;
	data_in = data_in << 1;
	data_in = (data_in ^ (data_in >> 4) ^ (data_in >> 8)) & 0x0f;
	
	return speed_data | data_in;
}
	

// [[maybe_unused]] static void transfer(uint16_t data_in){
// 	uint8_t i;
// 	for(i=0;i<16;i++)
// 	{
// 		if(data_in & 0x8000)data[i] = 171;
// 		else data[i] = 81;
// 		data_in = data_in << 1;
// 	}
// }



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

    auto & timer = hal::timer1;

    timer.init({
        .remap = hal::TIM1_REMAP_A8_A9_A10_A11__B13_B14_B15,
        // .count_freq = hal::NearestFreq(20_KHz),
        .count_freq = hal::NearestFreq(600_KHz),
        .count_mode = hal::TimerCountMode::Up
    }).unwrap()
        .alter_to_pins({
            hal::TimerChannelSelection::CH1,
            hal::TimerChannelSelection::CH2,
            hal::TimerChannelSelection::CH3,
        })
        .unwrap();
    TIM_DMACmd(TIM1, TIM_DMA_CC1, ENABLE);
    TIM_DMACmd(TIM1, TIM_DMA_CC2, ENABLE);
    timer.start();
    auto & oc1 = timer.oc<1>();
    auto & oc2 = timer.oc<2>();

    DShotChannel ch1{oc1};
    DShotChannel ch2{oc2};

    ch1.init();
    ch2.init();

    while(true){
        const auto now_secs = clock::seconds();

        const auto dutycycle = 0.4_iq16 * iq16(math::sinpu(4 * now_secs)) + 0.5_iq16;
        const auto dutycycle_int = static_cast<uint16_t>(600 * dutycycle);
        ch1.set_content(dutycycle_int);
        ch2.set_content(dutycycle_int);

        // clock::delay(1ms);
        while(oc1.dma().unwrap().pending_count() != 0){
            __nop;
        }


        // DEBUG_PRINTLN(
        //     // now_secs, 
        //     // oc1.cvr(), 
        //     // oc1.arr(), 
        //     // trig_cnt_, 
        //     // TIM3->CH1CVR,
        //     // TIM3->CH2CVR,
        //     // TIM3->CH3CVR,
        //     // cap_value_

        //     // is_up_edge,
        //     dutycycle_int,
        //     ch1.burst_dma_pwm_.is_done(),
        //     oc1.dma().unwrap().pending_count(),
        //     hal::usart2.available()

        // );
    }
}