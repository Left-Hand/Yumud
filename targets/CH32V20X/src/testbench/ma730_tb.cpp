#include "tb.h"
#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "bus/spi/spihw.hpp"

void ma730_main(){
    DEBUGGER.init(DEBUG_UART_BAUD, CommMethod::Blocking);

    spi1.init(9000000);
    spi1.bindCsPin(portA[15], 2);


    MA730 ma730{spi1, 2};
    ma730.init();

    while(true){
        DEBUG_PRINTLN(ma730.getLapPosition());
    }
}