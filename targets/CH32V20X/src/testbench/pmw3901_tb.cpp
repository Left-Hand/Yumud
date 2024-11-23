#include "tb.h"

#include "drivers/Proximeter/PMW3901/PMW3901.hpp"
#include "hal/bus/spi/spihw.hpp"


using namespace ymd::drivers;

void pmw3901_main(){

    DEBUGGER_INST.init(DEBUG_UART_BAUD, CommMethod::Blocking);

    // SpiSw spisw {SPI1_SCLK_GPIO, SPI1_MOSI_GPIO, SPI1_MISO_GPIO};

    auto & spi = spi1;


    spi.init(9_MHz);
    spi.bindCsPin(portA[15], 0);

    PMW3901 pmw{spi, 0};
    pmw.init();
    // pmw.update();
    
    Vector2_t<int16_t> pos;
    while(true){
        // pmw.verify();
        pmw.update();
        pos += Vector2(pmw.getMotion());
        auto [x,y] = pos;
        DEBUG_PRINTLN(x,y);
        delayMicroseconds(5000);
    }
}