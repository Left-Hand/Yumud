#include "testbench/tb.h"

int main(){
    Sys::Misc::prework();
    UartHw & logger = uart1;
    logger.init(115200 * 4);

    delay(200);
    pmdc_tb();
    // uart_tb(uart1);
    // stepper_tb(uart1);
    // lua_tb(uart1);
    // pwm_tb(logger);
    // adc_tb(logger);
    // temp_tb(logger);
    // temp_tb2(logger);
    // eeprom_tb(uart1);
    while(true);
}