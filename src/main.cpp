#include "testbench/tb.h"

int main(){
    Sys::Misc::prework();
    UartHw & logger = uart2;
    logger.init(921600, CommMethod::Blocking);

    delay(200);
    // stepper_tb(uart1);
    // lua_tb(uart1);
    pwm_tb(logger);
    // eeprom_tb(uart1);
    while(true);
}