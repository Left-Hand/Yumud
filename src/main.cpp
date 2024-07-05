#include "testbench/tb.h"

int main(){
    Sys::Misc::prework();

    #ifdef CH32V20X
    UartHw & logger = uart1;
    logger.init(115200 * 4);

    delay(200);
    // pmdc_tb();
    // uart_tb(uart1);
    // joystick_tb(uart1);
    gpio_tb(portC[13]);
    // stepper_tb(uart1);
    // lds14_tb(logger, uart2);-
    // pca_tb(logger);
    // lua_tb(uart1);p
    // pwm_tb(logger);
    // adc_tb(logger);
    // temp_tb(logger);
    // temp_tb2(logger);
    // eeprom_tb(uart1);

    #elif defined(CH32V30X)

    // UartHw & logger = uart2;
    UartHw & logger = uart8;
    // logger.init(115200 * 4, CommMethod::Dma, CommMethod::Interrupt);
    // logger.init(115200 * 4, CommMethod::Dma);
    
    logger.init(115200 * 4);
    delay(200);
    // pmdc_tb();
    // gpio_tb(portC[13]);
    // dma_tb(logger, dma1Ch1);
    // dma_tb(logger, UART7_RX_DMA_CH);
    uart_tb(logger);
    #endif
    while(true);
}