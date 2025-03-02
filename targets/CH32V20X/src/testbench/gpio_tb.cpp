#include "tb.h"
#include "sys/clock/clock.h"
void gpio_tb(hal::GpioIntf & gpio){
    gpio.outpp();
    while(true){
        gpio.set();
        delay(200);
        gpio.clr();
        delay(200);
    }
}