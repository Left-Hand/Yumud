#include "tb.h"
#include "drivers/LightSensor/TCS34725/tcs34725.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

using namespace ymd::drivers;

void tcs34725_tb(OutputStream & logger, I2c & i2c){
    TCS34725 tcs{i2c};
    tcs.init();
    tcs.startConv();
    while(true){
        logger.println(tcs.getCRGB());
        delay(30);
    }
}

void tcs34725_main(){
    auto & logger = DEBUGGER_INST;
    logger.init(DEBUG_UART_BAUD);
    I2cSw i2c{portD[0], portD[1]};
    i2c.init(100000);
    tcs34725_tb(logger, i2c);
}