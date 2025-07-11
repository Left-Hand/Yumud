#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/spi/spihw.hpp"

#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;


void ma730_main(){
    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);

    spi1.init({9_MHz});

    MA730 ma730{&spi1, spi1.allocate_cs_gpio(&portA[15]).unwrap()};
    ma730.init({
        .direction = CW
    }).unwrap();

    while(true){
        ma730.update().unwrap();
        DEBUG_PRINTLN(ma730.read_lap_position().unwrap());
    }
}