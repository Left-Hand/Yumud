#pragma once

#include <cstdint> 

namespace CH32X035{

struct [[nodiscard]] R32_RCC_CTLR{
    uint32_t HSION:1;
    uint32_t HSIRDY:1;
    uint32_t :1;
    uint32_t HSITRIM:5;
    uint32_t HSICAL:8;
    uint32_t :16;
};

struct [[nodiscard]] R32_RCC_CFGR0{
    uint32_t :4;
    uint32_t HPRE:4;
    uint32_t :16;
    uint32_t MCO:3;
    uint32_t :5;
};

struct [[nodiscard]] R32_RCC_APB2PRSTR{
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

struct [[nodiscard]] R32_RCC_APB1PRSTR{
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

struct [[nodiscard]] R32_RCC_APB2PCENR{
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

struct [[nodiscard]] R32_RCC_APB1PCENR{
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

struct [[nodiscard]] R32_RCC_AHBPCENR{
    uint32_t DMA1:1;
    uint32_t :1;
    uint32_t SRAM:1;
    uint32_t :9;
    uint32_t USBFS:9;
    uint32_t :4;

    uint32_t USBPD:1;
    uint32_t :14;
};

struct [[nodiscard]] R32_RCC_RSTSCKR{
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

struct [[nodiscard]] R32_RCC_AHBRSTR{
    uint32_t :12;
    uint32_t USBFSRST:1;
    uint32_t PIOCRST:1;
    uint32_t :3;
    uint32_t USBPDRST:1;
};

struct [[nodiscard]] RCC_Def{
    volatile R32_RCC_CTLR CTLR;
    volatile R32_RCC_CFGR0 CFGR0;
    volatile R32_RCC_APB2PRSTR APB2PRSTR;
    volatile R32_RCC_APB1PRSTR APB1PRSTR;
    volatile R32_RCC_AHBPCENR AHBPCENR;
    volatile R32_RCC_APB2PCENR APB2PCENR;
    volatile R32_RCC_APB1PCENR APB1PCENR;
    volatile R32_RCC_RSTSCKR RSTSCKR;

    constexpr void enable_hsi(const Enable en){
        CTLR.HSION = en;
    }

    constexpr bool is_hsi_ready(){
        return CTLR.HSIRDY;
    }

    constexpr void set_hsi_trim(const uint8_t trim){
        CTLR.HSITRIM = trim;
    }

    constexpr uint8_t get_hsi_cal(){
        return CTLR.HSICAL;
    }

    constexpr void set_ahb_prescale(const int prescale){
        CFGR0.HPRE = [&]{
            switch(prescale){
                default:
                case 1:   return 0b0000; 
                case 2:   return 0b0001; 
                case 3:   return 0b0010; 
                case 4:   return 0b0011; 
                case 5:   return 0b0100; 
                case 6:   return 0b0101; 
                case 7:   return 0b0110; 
                case 8:   return 0b1010; 


                case 16:  return 0b1011; 
                case 64:  return 0b1100; 
                case 128: return 0b1101; 
                case 256: return 0b1110; 
                case 512: return 0b1111; 
            }
        }();
    }

    constexpr void set_mco_source(const uint8_t source){
        CFGR0.MCO = source;
    }

    constexpr void reset_io_port(const uint8_t index){
        switch(index){
            case 0: APB2PRSTR.IOPARST = 1;
            case 1: APB2PRSTR.IOPBRST = 1;
            case 2: APB2PRSTR.IOPCRST = 1;
        }
    }

    constexpr void reset_timer(const uint8_t index){
        switch(index){
            case 2: APB1PRSTR.TIM2RST = 1;
            case 3: APB1PRSTR.TIM3RST = 1;
        }
    }

    constexpr void reset_adc(const uint8_t index){
        switch(index){
            case 1: APB2PRSTR.ADC1RST = 1;
        }
    }

    constexpr void reset_usart(const uint8_t index){
        switch(index){
            case 1: APB1PRSTR.USART2RST = 1;
            case 2: APB1PRSTR.USART2RST = 1;
            case 3: APB1PRSTR.USART3RST = 1;
            case 4: APB1PRSTR.USART4RST = 1;
        }
    }

    constexpr void reset_spi(const uint8_t index, const Enable en){
        switch(index){
            case 1: APB2PRSTR.SPI1RST = en;
        }
    }

    constexpr void reset_i2c(const uint8_t index){
        switch(index){
            case 1: APB1PRSTR.I2C1RST = 1;
        }
    }

    constexpr void reset_wwdg(){
        RSTSCKR.WWDG_RSTF = 1;
    }

    constexpr void reset_pwr(){
        RSTSCKR.POR_RSTF = 1;
    }

    constexpr void enable_dma_clock(const uint8_t index, const Enable en){
        switch(index){
            case 1: AHBPCENR.DMA1 = en;
        }
    }

    constexpr void enable_usbfs_clock(const Enable en){
        AHBPCENR.USBFS = en;
    }

    constexpr void enable_usbpd_clock(const Enable en){
        AHBPCENR.USBPD = en;
    }

    constexpr void enable_afio_clock(const Enable en){
        APB2PCENR.AFIOEN = en;
    }

    constexpr void enable_io_port_clock(const uint8_t index, const Enable en){
        switch(index){
            case 0: APB2PCENR.IOPAEN = en;
            case 1: APB2PCENR.IOPBEN = en;
            case 2: APB2PCENR.IOPCEN = en;
        }
    }

    constexpr void enable_timer_clock(const uint8_t index, const Enable en){
        switch(index){
            case 2: APB1PCENR.TIM2EN = en;
            case 3: APB1PCENR.TIM3EN = en;
        }
    }

    constexpr void enable_adc_clock(const uint8_t index, const Enable en){
        switch(index){
            case 1: APB2PCENR.ADC1EN = en;
        }
    }

    constexpr void enable_usart_clock(const uint8_t index, const Enable en){
        switch(index){
            case 1: APB1PCENR.USART2EN = en;
            case 2: APB1PCENR.USART3EN = en;
            case 3: APB1PCENR.USART4EN = en;
        }
    }

    constexpr void enable_spi_clock(const uint8_t index, const Enable en){
        switch(index){
            case 1: APB2PCENR.SPI1EN = en;
        }
    }

    constexpr void enable_i2c_clock(const uint8_t index, const Enable en){
        switch(index){
            case 1: APB1PCENR.I2C1EN = en;
        }
    }

    constexpr void enable_wwdg_clock(const Enable en){
        APB1PCENR.WWDGEN = en;
    }

    constexpr void enable_pwr_clock(const Enable en){
        APB1PCENR.PWREN = en;
    }

    
};

}