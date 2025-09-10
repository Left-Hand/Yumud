#include "src/testbench/tb.h"
#include "core/clock/clock.hpp"

#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::hal;
void gpio_tb(hal::GpioIntf & gpio){
    gpio.outpp();
    while(true){
        gpio.set();
        clock::delay(200ms);
        gpio.clr();
        clock::delay(200ms);
    }
}

void gpio_main(){
    auto gpio = hal::PC<13>();
    gpio_tb(gpio);
}