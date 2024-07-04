#include "testbench/tb.h"

int main(){
    Sys::Misc::prework();
    UartHw & logger = uart1;
    logger.init(115200 * 4);

    delay(200);
    // pmdc_tb();
    // uart_tb(uart1);
<<<<<<< HEAD
    // stepper_tb(uart1);
    filter_tb(uart1);
    // lds14_tb(logger, uart2);
=======
    // joystick_tb(uart1);
    gpio_tb(portC[13]);
    // stepper_tb(uart1);
    // lds14_tb(logger, uart2);-
>>>>>>> addv3
    // pca_tb(logger);
    // lua_tb(uart1);p
    // pwm_tb(logger);
    // adc_tb(logger);
    // temp_tb(logger);
    // temp_tb2(logger);
    // eeprom_tb(uart1);
    while(true);
}