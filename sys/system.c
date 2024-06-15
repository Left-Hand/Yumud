#include "system.h"
#include "sys/kernel/clock.h"

void prework(void){
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    Systick_Init();

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE );
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE );

    #ifdef HAVE_GPIOD
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE );
    GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);
    #endif
    PWR_BackupAccessCmd( ENABLE );
    RCC_LSEConfig( RCC_LSE_OFF );
    BKP_TamperPinCmd(DISABLE);
    PWR_BackupAccessCmd(DISABLE);
}