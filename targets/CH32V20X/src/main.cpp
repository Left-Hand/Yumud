#include "testbench/tb.h"
#include "app/embd/embd.h"
#include "app/smc/smc.h"


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
    preinit();

    #ifdef CH32V20X

    // eeprom_main();
    // uart_main();
    // qlz_main();
    // embd_main();

    // delay(200);
    // pmdc_tb();
    // uart_tb(uart1);
    // joystick_tb(uart1);
    // gpio_tb(portC[13]);
    // can_tb(logger, can1, false);
    // stepper_tb(uart1);
    // lds14_tb(logger, uart2);
    // pca_tb(logger);
    // lua_tb(uart1);p
    // pwm_tb(logger);
    // adc_tb(logger);
    // temp_tb(logger);
    // temp_tb2(logger);
    // eeprom_tb(uart1);

    #elif defined(CH32V30X)

    // gpio_tb(UART7_TX_GPIO);
    // uart_main();
    // calc_main();
    // dshot_main();
    // usbcdc_tb();
    // embd_main();
    smc_main();
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
    // embd_main();
    // can_tb(logger, can1);
    #endif
    while(true);
}