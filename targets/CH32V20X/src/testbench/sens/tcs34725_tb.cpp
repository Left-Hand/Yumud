#include "src/testbench/tb.h"

#include "sys/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "drivers/LightSensor/TCS34725/tcs34725.hpp"


using namespace ymd::drivers;

void tcs34725_tb(OutputStream & logger, hal::I2c & i2c){
    TCS34725 tcs{i2c};
    tcs.init();
    tcs.startConv();
    while(true){
        logger.println(tcs.getCRGB());
        delay(30);
    }
}

void tcs34725_main(){
    DEBUGGER_INST.init(DEBUG_UART_BAUD);
    DEBUGGER.retarget(&DEBUGGER_INST);
    I2cSw i2c{portD[0], portD[1]};
    i2c.init(100000);
    tcs34725_tb(DEBUGGER, i2c);
}