#pragma once

#include <cstdint>

namespace hwspec::CH32V20x::AFIO_Regs{

struct ECR_Reg{
    uint32_t PIN:4;
    uint32_t PORT:3;
    uint32_t :1;
    uint32_t :24;
};

struct PCFG1_Reg{
    uint32_t SPI1_RM:1;
    uint32_t I2C1_RM:1;
    uint32_t USART1_RM:1;
    uint32_t USART2_RM:1;
    uint32_t USART3_RM:2;
    uint32_t TIM1_RM:2;
    uint32_t TIM2_RM:2;
    uint32_t TIM3_RM:2;
    uint32_t TIM4_RM:1;
    uint32_t CAN1_RM:2;
    uint32_t PDO1_RM:2;
    uint32_t TIM5CH4_RM:1;
    uint32_t ADC1_ETRGINJ:1;
    uint32_t ADC1_ETRGREJ:1;
    uint32_t ADC2_ETRGINJ:1;
    uint32_t ADC2_ETRGREJ:1;
    uint32_t ETH_RM:1;
    uint32_t CAN2_RM:1;
    uint32_t MIIRMII_SEL:1;

    uint32_t SW_CFG:3;
    uint32_t :1;
    uint32_t SPI3_RM:1;
    uint32_t TIM2_ITR1_RM:1;
    uint32_t PTP_PPS_RM:1;
    uint32_t :1;
};

struct PCFG2_Reg{
    uint32_t TIM8_RM:2;
    uint32_t USART6_RM:2;
    uint32_t USART7_RM:2;
    uint32_t USART8_RM:2;
    uint32_t TIM9_RM:2;
    uint32_t TIM10_RM:2;
    uint32_t TIM11_RM:2;
};


struct EXTICR1_Reg{
    uint32_t EXTI0:4;
    uint32_t EXTI1:4;
    uint32_t EXTI2:4;
    uint32_t EXTI3:4;
    uint32_t :16;
};

struct EXTICR2_Reg{
    uint32_t EXTI4:4;
    uint32_t EXTI5:4;
    uint32_t EXTI6:4;
    uint32_t EXTI7:4;
    uint32_t :16;
};

struct EXTICR3_Reg{
    uint32_t EXTI8:4;
    uint32_t EXTI9:4;
    uint32_t EXTI10:4;
    uint32_t EXTI11:4;
    uint32_t :16;
};

struct EXTICR4_Reg{
    uint32_t EXTI12:4;
    uint32_t EXTI13:4;
    uint32_t EXTI14:4;
    uint32_t EXTI15:4;
    uint32_t :16;
};

struct PCFR2_Reg{
    uint32_t :2;
    uint32_t TIM8_RM:1;
    uint32_t TIM9_RM:2;
    uint32_t TIM10_RM:2;
    uint32_t :3;
    uint32_t FSMC_NADV:1;
    uint32_t :5;
    uint32_t USART4_RM:2;
    uint32_t USART5_RM:2;
    uint32_t USART6_RM:2;
    uint32_t USART7_RM:2;
    uint32_t USART8_RM:2;
    uint32_t USART1_RM1:2;
    uint32_t :5;
};

}