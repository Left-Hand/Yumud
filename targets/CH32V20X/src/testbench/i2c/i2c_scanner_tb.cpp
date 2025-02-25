#include "sys/debug/debug.hpp"

#include "src/testbench/tb.h"
#include "hal/bus/i2c/i2csw.hpp"
#include <bitset>

void i2c_scanner_main(){
    auto & logger = DEBUGGER_INST;
    logger.init(DEBUG_UART_BAUD);
    
    I2cSw i2c = {portD[2], portC[12]};
    i2c.init(100_KHz);

    {
        bool found = false;
        for(uint8_t i = 0; i < 128; i++){
            auto err = i2c.begin(i << 1);
            if(err.ok()){
                DEBUG_PRINTS("Found device at address: ", std::bitset<8>(i << 1));

                found = true;
            }
            i2c.end();
        }

        if(found == false){
            DEBUG_PRINTLN("No device found");
        }
    }

    while(true);
}