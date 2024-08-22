#include "tb.h"

#include "drivers/Wireless/Radio/LT8920/lt8920.hpp"
#include "bus/spi/spihw.hpp"
#include "bus/spi/spisw.hpp"

void lt8920_main(){
    DEBUGGER.init(DEBUG_UART_BAUD, CommMethod::Blocking);


    SpiSw spisw {SPI1_SCLK_GPIO, SPI1_MOSI_GPIO, SPI1_MISO_GPIO};

    auto & spi = spi1;
    // auto & spi = spisw;


    spi.init(1_MHz);
    spi.bindCsPin(portA[0], 0);

    LT8920 lt{spi, 0};


    if(lt.verify()){
        DEBUG_PRINTLN("LT8920 founded");
    }else{
        exit(1);
    }

    lt.init();
    lt.setDataRate(1_MHz);

    auto src = String("Hello World!!!");
    while(true){
    // lt.verify();
        // lt.setDataRate(LT8920::DataRate::Kbps125);
        // DEBUG_PRINTLN(lt.isRfSynthLocked());
        lt.writeBlock((const uint8_t *)src.c_str(), src.length());
        delay(200);
    }
}