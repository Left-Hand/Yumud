#include "tb.h"
#include "hal/bus/i2c/i2csw.hpp"
#include "drivers/Proximeter/PAJ7620/paj7620.hpp"

using namespace ymd::drivers;

void paj7620_main(){
    auto & logger = DEBUGGER_INST;
    logger.init(DEBUG_UART_BAUD);
    
    I2cSw i2c = {portD[2], portC[12]};


    
    i2c.init(100_KHz);
    PAJ7620 paj{i2c};

    // while(true){
    //     servo_left.setRadian(0);
    //     servo_right.setRadian(0);
    // }
}