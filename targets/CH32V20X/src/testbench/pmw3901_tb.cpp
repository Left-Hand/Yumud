#include "tb.h"

#include "drivers/Proximeter/PMW3901/PMW3901.hpp"
#include "hal/bus/spi/spihw.hpp"


using namespace ymd::drivers;

void pmw3901_main(){

    DEBUGGER_INST.init(DEBUG_UART_BAUD, CommMethod::Blocking);

    // SpiSw spisw {SPI1_SCLK_GPIO, SPI1_MOSI_GPIO, SPI1_MISO_GPIO};

    auto & spi = spi1;


    spi.init(1_KHz);
    spi.bindCsPin(portA[15], 0);

    PMW3901 pmw{spi, 0};
    // pmw.init();
    
    while(true){
        pmw.verify();
        // auto [x,y] = pmw.getMotion();
        // DEBUG_PRINTLN(x,y);
    }
}