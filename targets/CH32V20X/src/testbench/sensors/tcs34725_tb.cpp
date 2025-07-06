#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/HumitureSensor/TCS34725/tcs34725.hpp"
#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

void tcs34725_tb(OutputStream & logger, hal::I2c & i2c){
    TCS34725 tcs{&i2c};
    tcs.init({}).examine();
    tcs.start_conv().examine();
    while(true){
        logger.println(tcs.get_crgb());
        clock::delay(30ms);
    }
}

void tcs34725_main(){
    DEBUGGER_INST.init({576000});
    DEBUGGER.retarget(&DEBUGGER_INST);
    I2cSw i2c{&portD[0], &portD[1]};
    i2c.init(100000);
    tcs34725_tb(DEBUGGER, i2c);
}