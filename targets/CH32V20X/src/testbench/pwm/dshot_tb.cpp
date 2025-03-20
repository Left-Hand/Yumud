#include "src/testbench/tb.h"

#include "core/clock/clock.h"
#include "core/debug/debug.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/timer/timer_oc.hpp"

#include "drivers/Modem/dshot/dshot.hpp"

scexpr size_t n = 40;
static std::array<uint16_t, 40> data;

using namespace ymd::drivers;
using namespace ymd;

[[maybe_unused]] static uint16_t m_crc(uint16_t data_in){
	uint16_t speed_data;
	speed_data = data_in << 5;
	data_in = data_in << 1;
	data_in = (data_in ^ (data_in >> 4) ^ (data_in >> 8)) & 0x0f;
	
	return speed_data | data_in;
}
	

[[maybe_unused]] static void transfer(uint16_t data_in){
	uint8_t i;
	for(i=0;i<16;i++)
	{
		if(data_in & 0x8000)data[i] = 171;
		else data[i] = 81;
		data_in = data_in << 1;
	}
}


[[maybe_unused]] static void dshot_tb_new(OutputStream & logger, TimerOC & oc1, TimerOC & oc2){
    DShotChannel ch1{oc1};
    DShotChannel ch2{oc2};

    ch1.init();
    ch2.init();


    auto entry = millis();
    while(millis() - entry < 3000){
        // ch1.enable();
        // ch2.enable();
        ch1 = 0;
        ch2 = 0;

        delay(20);
    }

    delay(200);
    // delay(3000);
    // ch1.init();
    // ch2.init();

    // oc1.init();
    // oc2.init();
    // constexpr real_t base = 0.12;
    // constexpr real_t full = 0.95;
    // constexpr real_t delta = full-  base;
    // ch1 = base;
    // ch2 = base;

    // auto t0 = t;
    // real_t temp = base;
    // while(true){
        // ch2 = 0.2;
        // delay(15);

        // temp = std::max(temp, base + (0.5 + 0.5 * sin((t - t0)/2)) * delta);
        // ch1 = temp;
        // ch2 = temp;
        // delay(20);

        // delay(10);
        // ch1 = 0.6;
        // ch2 = 0.6;
        // oc2 = 0.4;
        // logger.println(temp);
        // delay(20);
    // }
}

void dshot_main(){

    DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);
    DEBUGGER.retarget(&DEBUGGER_INST);

    DEBUGGER.setRadix(10);
    DEBUGGER.setEps(4);
    AdvancedTimer & timer = timer1;

    // timer.enableArrSync();
    // timer.enableCvrSync();


    timer.init(234, 1);
    auto & oc = timer.oc(1);
    auto & oc2 = timer.oc(2);

    dshot_tb_new(DEBUGGER,oc, oc2);
    // dshot_tb_old(logger,oc, oc2);

    // while(true);
}