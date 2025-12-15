#include "src/testbench/tb.h"
#include "core/clock/clock.hpp"

#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

void gpio_tb(hal::GpioIntf & gpio){
    gpio.outpp();
    while(true){
        gpio.set_high();
        clock::delay(200ms);
        gpio.set_low();
        clock::delay(200ms);
    }
}

void gpio_main(){
    auto gpio = hal::PC<13>();
    gpio_tb(gpio);
}