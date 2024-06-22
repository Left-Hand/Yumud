#include "testbench/tb.h"

int main(){
    Sys::Misc::prework();
    UartHw & logger = uart1;
    logger.init(115200 * 4);
    uart2.init(115200 * 4);

    delay(200);
    // pmdc_tb();
    // uart_tb(uart2);
    // stepper_tb(uart1);
    // lds14_tb(logger, uart2);
    pca_tb(logger);
    // lua_tb(uart1);
    // pwm_tb(logger);
    // adc_tb(logger);
    // temp_tb(logger);
    // temp_tb2(logger);
    // eeprom_tb(uart1);
    while(true);
}