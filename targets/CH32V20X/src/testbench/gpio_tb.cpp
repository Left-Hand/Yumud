#include "tb.h"
#include "sys/clock/clock.h"
void gpio_tb(GpioConcept & gpio){
    gpio.outpp();
    while(true){
        gpio.set();
        delay(200);
        gpio.clr();
        delay(200);
    }
}