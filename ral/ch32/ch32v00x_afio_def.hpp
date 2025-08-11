#pragma once

#include <cstdint>

namespace ymd::ral::CH32V00x::AFIO_Regs{


struct EXTICR_Reg{
    uint32_t EXTI0:2;
    uint32_t EXTI1:2;
    uint32_t EXTI2:2;
    uint32_t EXTI3:2;
    uint32_t EXTI4:2;
    uint32_t EXTI5:2;
    uint32_t EXTI6:2;
    uint32_t EXTI7:2;
    uint32_t :16;
};

struct PCFR1_Reg{
    uint32_t SPI1_RM:3;
    uint32_t I2C1_RM:3;
    uint32_t USART1_RM:4;
    uint32_t TIM1_RM:4;
    uint32_t TIM2_RM:3;
    uint32_t PA1PA2_RM:3;
    uint32_t ADC_ETRGING_RM:3;
    uint32_t ADC_ETRGREG_RM:3;
    uint32_t USART2_RM:3;
    uint32_t :1;
    uint32_t SWCFG:3;
    uint32_t :5;
};


}