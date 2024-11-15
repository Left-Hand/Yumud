#include "tb.h"
#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "bus/spi/spihw.hpp"


using namespace ymd::drivers;
void ma730_main(){
    DEBUGGER_INST.init(DEBUG_UART_BAUD, CommMethod::Blocking);

    spi1.init(9000000);
    spi1.bindCsPin(portA[15], 2);


    MA730 ma730{spi1, 2};
    ma730.init();

    while(true){
        ma730.update();
        DEBUG_PRINTLN(ma730.getLapPosition());
    }
}