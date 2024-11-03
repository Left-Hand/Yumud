#include "tb.h"
#include "drivers/LightSensor/TCS34725/tcs34725.hpp"

using namespace yumud::drivers;

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
    auto & logger = DEBUGGER;
    logger.init(DEBUG_UART_BAUD);
    I2cSw i2c{portD[0], portD[1]};
    i2c.init(100000);
    tcs34725_tb(logger, i2c);
}