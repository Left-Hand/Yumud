#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/spi/spihw.hpp"

#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd::drivers;
void ma730_main(){
    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);

    spi1.init(9_MHz);

    MA730 ma730{spi1, spi1.attach_next_cs(portA[15]).value()};
    ma730.init();

    while(true){
        ma730.update();
        DEBUG_PRINTLN(ma730.get_lap_position().unwrap());
    }
}