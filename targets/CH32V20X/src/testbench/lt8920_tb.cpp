#include "tb.h"

#include "drivers/Wireless/Radio/LT8920/lt8920.hpp"
#include "bus/spi/spihw.hpp"

void lt8920_main(){
    DEBUGGER.init(DEBUG_UART_BAUD, CommMethod::Blocking);

    spi1.init(18000000);
    spi1.bindCsPin(portA[0], 0);

    LT8920 lt{spi1, 0};
    lt.init();

    while(true){
        DEBUG_PRINTLN(millis());
    }
}