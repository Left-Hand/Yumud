#include "testbench/tb.h"
#include "app/embd/embd.h"
#include "app/smc/smc.h"


void preinit(){
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    Systick_Init();

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE );
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE );
    PWR_BackupAccessCmd( ENABLE );
    RCC_LSEConfig( RCC_LSE_OFF );
    BKP_TamperPinCmd(DISABLE);
    PWR_BackupAccessCmd(DISABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOD, ENABLE);
}


[[maybe_unused]] static void uart_tb_old(){

    USART_InitTypeDef USART_InitStructure = {0};
    GPIO_InitTypeDef  GPIO_InitStructure = {0};

    /* USART2 TX-->A.2   RX-->A.3 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    
    /* USART2 TX-->A.2   RX-->A.3 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7, ENABLE);

    GPIO_PinRemapConfig(GPIO_FullRemap_USART7, ENABLE);


    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(UART7, &USART_InitStructure);
    
    USART_Cmd(UART7, ENABLE);
    [[maybe_unused]] auto write = [](const char * str){
        for(size_t i=0;str[i];i++){
            UART7->DATAR = (uint8_t)str[i];
            while((UART7->STATR & USART_FLAG_TXE) == RESET);
        }
    };
    
    while(true){
        write("hello\r\n");
    }
}

int main(){
    preinit();
    uart_tb_old();

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
    // embd_main();
    // can_tb(logger, can1);
    #endif
    while(true);
}