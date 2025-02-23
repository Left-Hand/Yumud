

#include <examples/exmaples.hpp>
#include <hal/bus/i2c/i2csw.hpp>


namespace exmaples{
    
void i2c_scanner(){
    DEBUGGER.init(DEBUGGER_INST, DEBUG_UART_BAUD);

    I2cSw i2c = {portD[2], portC[12]};
    i2c.init(100_KHz);

    {
        bool found = false;
        for(uint8_t i = 0; i < 128; i++){
            auto err = i2c.begin(i << 1);
            if(!err){
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

}