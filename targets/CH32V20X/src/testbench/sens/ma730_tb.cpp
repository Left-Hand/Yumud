#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/spi/spihw.hpp"

#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"


using namespace ymd::drivers;
void ma730_main(){
    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);

    spi1.init(9000000);
    spi1.bind_cs_pin(portA[15], 2);


    MA730 ma730{spi1, 2};
    ma730.init();

    while(true){
        ma730.update();
        DEBUG_PRINTLN(ma730.getLapPosition());
    }
}