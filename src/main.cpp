#include "testbench/tb.h"

int main(){
    Sys::Misc::prework();
    uart1.init(921600, CommMethod::Dma, CommMethod::Blocking);
    delay(200);
    uart1.println("power on");
    stepper_tb(uart1);
    // eeprom_tb(uart1);
    while(true);
}