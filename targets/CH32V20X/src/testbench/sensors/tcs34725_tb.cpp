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

#define SCL_GPIO hal::PD<0>()
#define SDA_GPIO hal::PD<1>()
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

    auto scl_gpio_ = SCL_GPIO;
    auto sda_gpio_ = SDA_GPIO;

    I2cSw i2c{&scl_gpio_, &sda_gpio_};
    i2c.init(100000);
    tcs34725_tb(DEBUGGER, i2c);
}