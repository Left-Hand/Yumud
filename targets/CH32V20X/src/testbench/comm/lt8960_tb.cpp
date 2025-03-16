#include "src/testbench/tb.h"

#include "sys/debug/debug.hpp"


#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/wireless/Radio/LT8960/LT8960L.hpp"

using namespace ymd::drivers;


#define UART uart2

#define MAG_ACTIVATED

void lt8960_tb(hal::I2c & i2c){

    auto & led = portC[13];
    led.outpp();

    LT8960L ltr{&i2c};

    // ltr.verify().unwrap();

    while(true){
        DEBUG_PRINTLN("verify status", ltr.verify().is_ok());
        
        delay(50);
        led.toggle();
    }
}

void lt8960_main(){
    UART.init(576_KHz);
    DEBUGGER.retarget(&UART);
    DEBUGGER.noBrackets();



    I2cSw i2c{portB[6], portB[7]};
    i2c.init(400_KHz);

    delay(200);

    lt8960_tb(i2c);
}