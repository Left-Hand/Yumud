#include "sys/core/system.hpp"
#include "app/embd/embd.h"

#include "app/wlsy/wlsy_inc.hpp"
#include "hal/gpio/port_virtual.hpp"
#include "sys/debug/debug_inc.h"
#include "robots/foc/bldc/bldc.hpp"

#include "testbench/tb.h"


void preinit(){
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    Systick_Init();

    GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE );
    PWR_BackupAccessCmd( ENABLE );
    RCC_LSEConfig( RCC_LSE_OFF );
    BKP_TamperPinCmd(DISABLE);
    PWR_BackupAccessCmd(DISABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOD, ENABLE);
}


int main(){

    #ifdef CH32V20X
    Sys::Misc::prework();
    // eeprom_main();
    // uart_main();
    // qlz_main();
    // embd_main();
    // embd_main();
    // bldc_main();
    // uart_main();
    // wlsy_main();
    // lt8920_main();
    // ma730_main();
    // bmi160_main();
    // delay(200);
    // pmdc_tb();
    // uart_tb(uart1);
    // joystick_tb(uart1);
    // gpio_tb(portC[13]);
    // can_tb(logger, can1, false);
    stepper_tb(uart1);
    // math_tb(uart1);
    // zdt_main(uart1);
    // m3508_main();
    // while(true);
    // lds14_tb(logger, uart2);
    // pca_tb(logger);
    // lua_tb(uart1);p
    // pwm_tb(logger);
    // adc_tb(logger);
    // temp_tb(logger);
    // temp_tb2(logger);
    // eeprom_tb(uart1);
    // eeprom_main();

    #elif defined(CH32V30X)
    preinit();

    // gpio_tb(UART7_TX_GPIO);
    // eeprom_main();
    // uart_main();
    // calc_main();
    // dshot_main();
    // usbcdc_tb();
    // embd_main();
    // w25qxx_main();
    // smc_main();
    // UartHw & logger = uart2;
    // logger.init(115200);
    // can_tb(logger, can1, true);
    // UartHw & logger = uart8;
    // logger.init(115200 * 4, CommMethod::Dma, CommMethod::Interrupt);
    // logger.init(115200 * 4, CommMethod::Dma);
// rng.init();
    // pmdc_tb();
    // gpio_tb(portC[13]);
    // dma_tb(logger, dma1Ch1);
    // dma_tb(logger, UART7_RX_DMA_CH);
    // uart_tb(logger);

    // uart7.init(1000000);
    // uart7.init(9600);
    // uart7.init(19200);

    // uart7.init(115200);
    // uart7.init(57600);
    // ch9141_tb(logger, uart7);
    // st77xx_tb(logger, spi2);
    embd_main();
    // can_tb(logger, can1);
    #endif
    while(true);
}