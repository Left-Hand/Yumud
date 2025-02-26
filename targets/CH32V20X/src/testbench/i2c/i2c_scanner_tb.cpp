#include "sys/debug/debug.hpp"

#include "src/testbench/tb.h"
#include "hal/bus/i2c/i2csw.hpp"
#include <bitset>

using namespace ymd;
void i2c_scanner_main(){
    auto & logger = uart2;
    logger.init(DEBUG_UART_BAUD);
    DEBUGGER.retarget(&logger);
    
    I2cSw i2c = {portB[13], portB[12]};
    i2c.init(100_KHz);

    {
        bool found = false;
        for(uint8_t i = 0; i < 128; i++){
            auto err = i2c.begin(i << 1);
            if(err.ok()){
                DEBUG_PRINTS("Found device at address: ", std::bitset<8>(i << 1), err);

                found = true;
            }
            i2c.end();

            delay(10);
        }

        if(found == false){
            DEBUG_PRINTLN("No device found");
        }
    }

    while(true);
}