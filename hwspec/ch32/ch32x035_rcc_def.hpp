#pragma once

#include <cstdint> 

namespace hwspec::CH32V20x::RCC_Regs{

struct CTLR_Reg{
    uint32_t HSION:1;
    uint32_t HSIRDY:1;
    uint32_t :1;
    uint32_t HSITRIM:5;
    uint32_t HSICAL:8;
    uint32_t :16;
};

struct CFGR0_Reg{
    uint32_t :4;
    uint32_t HPRE:4;
    uint32_t :16;
    uint32_t MCO:3;
    uint32_t :5;
};

struct APB2PRSTR_Reg{
    uint32_t AFIORST:1;
    uint32_t :1;

    uint32_t IOPARST:1;
    uint32_t IOPBRST:1;
    uint32_t IOPCRST:1;
    uint32_t :4;

    uint32_t ADC1RST:1;
    uint32_t :1;
    uint32_t TIM1RST:1;
    uint32_t SPI1RST:1;
    uint32_t :1;
    uint32_t USART1RST:1;
    uint32_t :17;
};

struct APB1PRSTR_Reg{
    uint32_t TIM2RST:1;
    uint32_t TIM3RST:1;
    uint32_t :9;
    uint32_t WWDGRST:1;
    uint32_t :5;
    uint32_t USART2RST:1;
    uint32_t USART3RST:1;
    uint32_t USART4RST:1;
    uint32_t :1;
    uint32_t I2C1RST:1;
    uint32_t :6;
    uint32_t PWRRST:1;
    uint32_t :3;
};

struct APB2PCENR_Reg{
    uint32_t AFIOEN:1;
    uint32_t :1;

    uint32_t IOPAEN:1;
    uint32_t IOPBEN:1;
    uint32_t IOPCEN:1;
    uint32_t :4;

    uint32_t ADC1EN:1;
    uint32_t :1;
    uint32_t TIM1EN:1;
    uint32_t SPI1EN:1;
    uint32_t :1;
    uint32_t USART1EN:1;
    uint32_t :17;
};

struct APB1PCENR_Reg{
    uint32_t TIM2EN:1;
    uint32_t TIM3EN:1;
    uint32_t :9;
    uint32_t WWDGEN:1;
    uint32_t :5;
    uint32_t USART2EN:1;
    uint32_t USART3EN:1;
    uint32_t USART4EN:1;
    uint32_t :1;
    uint32_t I2C1EN:1;
    uint32_t :6;
    uint32_t PWREN:1;
    uint32_t :3;
};

struct RSTSCKR_Reg{
    uint32_t :24;
    uint32_t RMVF:1;
    uint32_t OPA_RSTF:1;
    uint32_t PIN_RSTF:1;
    uint32_t POR_RSTF:1;
    uint32_t SFT_RSTF:1;
    uint32_t IWDG_RSTF:1;
    uint32_t WWDG_RSTF:1;
    uint32_t LPWR_RSTF:1;
};

struct AHBRSTR_Reg{
    uint32_t :12;
    uint32_t USBFSRST:1;
    uint32_t PIOCRST:1;
    uint32_t :3;
    uint32_t USBPDRST:1;
};

struct RCC_Def{
    volatile CTLR_Reg CTLR;
    volatile CFGR0_Reg CFGR0;
    volatile APB2PRSTR_Reg APB2PRSTR;
    volatile APB1PRSTR_Reg APB1PRSTR;
    volatile APB2PCENR_Reg APB2PCENR;
    volatile APB1PCENR_Reg APB1PCENR;
    volatile RSTSCKR_Reg RSTSCKR;
};

}