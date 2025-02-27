#pragma once

#include <cstdint> 

#define _SIZE(x) (sizeof(x))

#ifndef ASSERT_REG_IS_32BIT
#define ASSERT_REG_IS_32BIT(name) static_assert((sizeof(name) == 4),  #name " is not 4 bytes"); 
#endif

namespace CH32V20x{

struct R32_RCC_CTLR{
    
    uint32_t HSION:1;
    uint32_t HSIRDY:1;
    uint32_t :1;
    uint32_t HSITRIM:5;
    uint32_t HSICAL:8;
    uint32_t HSEON:1;
    uint32_t HSERDY:1;
    uint32_t HSEBYP:1;
    uint32_t CSSON:1;
    uint32_t :4;
    uint32_t PLLON:1;
    uint32_t PLLRDY:1;
    uint32_t PLL2ON:1;
    uint32_t PLL2RDY:1;
    uint32_t PLL3ON:1;
    uint32_t PLL3RDY:1;
    uint32_t :2;
};ASSERT_REG_IS_32BIT(R32_RCC_CTLR)

struct R32_RCC_CFGR0{
    uint32_t SW:2;
    uint32_t SWS:2;
    uint32_t HPRE:4;

    uint32_t PPRE1:3;
    uint32_t PPRE2:3;
    uint32_t ADC_PRE:2;

    uint32_t PLL_SRC:1;
    uint32_t PLL_XTPRE:1;
    uint32_t PLL_MUL:4;
    uint32_t USB_PRE:2;
    
    uint32_t ETH_PRE:1;
    uint32_t MCO:3;
    uint32_t :2;
    uint32_t ADC_DUTY:1;
};ASSERT_REG_IS_32BIT(R32_RCC_CFGR0)


struct R32_RCC_INTR{
    uint32_t LSI_RDYF:1;
    uint32_t LSE_RDYF:1;
    uint32_t HSI_RDYF:1;
    uint32_t HSE_RDYF:1;
    uint32_t PLL_RDYF:1;
    uint32_t PLL2_RDYF:1;
    uint32_t PLL3_RDYF:1;
    uint32_t CSSF:1;

    uint32_t LSI_RDYIE:1;
    uint32_t LSE_RDYIE:1;
    uint32_t HSI_RDYIE:1;
    uint32_t HSE_RDYIE:1;
    uint32_t PLL_RDYIE:1;
    uint32_t PLL2_RDYIE:1;
    uint32_t PLL3_RDYIE:1;
    uint32_t :1;

    uint32_t LSI_RDYC:1;
    uint32_t LSE_RDYC:1;
    uint32_t HSI_RDYC:1;
    uint32_t HSE_RDYC:1;
    uint32_t PLL_RDYC:1;
    uint32_t PLL2_RDYC:1;
    uint32_t PLL3_RDYC:1;
    uint32_t CSSC:1;

    uint32_t :8;
};ASSERT_REG_IS_32BIT(R32_RCC_INTR)

struct R32_RCC_APB2PRSTR{
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
};ASSERT_REG_IS_32BIT(R32_RCC_APB2PRSTR)

struct R32_RCC_APB1PRSTR{
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
};ASSERT_REG_IS_32BIT(R32_RCC_APB1PRSTR)

struct R32_RCC_AHBPCENR{
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
};ASSERT_REG_IS_32BIT(R32_RCC_AHBPCENR)


struct R32_RCC_APB2PCENR{
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
};ASSERT_REG_IS_32BIT(R32_RCC_APB2PCENR)

struct R32_RCC_APB1PCENR{
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
};ASSERT_REG_IS_32BIT(R32_RCC_APB1PCENR)

struct R32_RCC_RSTSCKR{
    uint32_t :24;
    uint32_t RMVF:1;
    uint32_t OPA_RSTF:1;
    uint32_t PIN_RSTF:1;
    uint32_t POR_RSTF:1;
    uint32_t SFT_RSTF:1;
    uint32_t IWDG_RSTF:1;
    uint32_t WWDG_RSTF:1;
    uint32_t LPWR_RSTF:1;
};ASSERT_REG_IS_32BIT(R32_RCC_RSTSCKR)

struct R32_RCC_AHBRSTR{
    uint32_t :12;
    uint32_t USBFSRST:1;
    uint32_t PIOCRST:1;
    uint32_t :3;
    uint32_t USBPDRST:1;
};ASSERT_REG_IS_32BIT(R32_RCC_AHBRSTR)


struct R32_RCC_APB2PRSTR{
    uint32_t AFIO_RST:1;
    uint32_t :1;
    uint32_t IOPA_RST:1;
    uint32_t IOPB_RST:1;
    uint32_t IOPC_RST:1;
    uint32_t IOPD_RST:1;
    uint32_t IOPE_RST:1;
    uint32_t :2;       

    uint32_t ADC1_RST:1;   
    uint32_t ADC2_RST:1;   
    uint32_t TIM1_RST:1;   
    uint32_t SPI1_RST:1;   
    uint32_t TIM8_RST:1;   
    uint32_t USART1_RST:1;   
    uint32_t :4;
    
    uint32_t TIM9_RST:1;
    uint32_t TIM10_RST:1; 
    uint32_t :11;        
};ASSERT_REG_IS_32BIT(R32_RCC_APB2PRSTR)

struct R32_RCC_APB1PRSTR{   
    uint32_t TIM2_RST:1;   
    uint32_t TIM3_RST:1;   
    uint32_t TIM4_RST:1;   
    uint32_t TIM5_RST:1;   
    uint32_t TIM6_RST:1;  
    uint32_t TIM7_RST:1;  
    uint32_t USART6_RST:1;  
    uint32_t USART7_RST:1;  
    uint32_t USART8_RST:1;  
    uint32_t :2;  
    uint32_t WWDG_RST:1;  
    uint32_t :2;  
    uint32_t SPI2_RST:1;  
    uint32_t SPI3_RST:1;  

    uint32_t :1;  
    uint32_t USART2_RST:1;  
    uint32_t USART3_RST:1; 
    uint32_t USART4_RST:1;  
    uint32_t USART5_RST:1; 
    uint32_t I2C1_RST:1;
    uint32_t I2C2_RST:1;
    uint32_t USBD_RST:1;

    uint32_t :1;
    uint32_t CAN1_RST:1;
    uint32_t CAN2_RST:1;
    uint32_t BKP_RST:1;
    uint32_t PWR_RST:1;
    uint32_t DAC_RST:1;
    uint32_t :2;
};ASSERT_REG_IS_32BIT(R32_RCC_APB1PRSTR)


struct R32_RCC_APB2PCENR{
    uint32_t AFIO_EN:1;
    uint32_t :1;
    uint32_t IOPA_EN:1;
    uint32_t IOPB_EN:1;
    uint32_t IOPC_EN:1;
    uint32_t IOPD_EN:1;
    uint32_t IOPE_EN:1;
    uint32_t :2;       

    uint32_t ADC1_EN:1;   
    uint32_t ADC2_EN:1;   
    uint32_t TIM1_EN:1;   
    uint32_t SPI1_EN:1;   
    uint32_t TIM8_EN:1;   
    uint32_t USART1_EN:1;   
    uint32_t :4;
    
    uint32_t TIM9_EN:1;
    uint32_t TIM10_EN:1; 
    uint32_t :11;        
};ASSERT_REG_IS_32BIT(R32_RCC_APB2PCENR)

struct R32_RCC_APB1PCENR{   
    uint32_t TIM2_EN:1;   
    uint32_t TIM3_EN:1;   
    uint32_t TIM4_EN:1;   
    uint32_t TIM5_EN:1;   
    uint32_t TIM6_EN:1;  
    uint32_t TIM7_EN:1;  
    uint32_t USART6_EN:1;  
    uint32_t USART7_EN:1;  
    uint32_t USART8_EN:1;  
    uint32_t :2;  
    uint32_t WWDG_EN:1;  
    uint32_t :2;  
    uint32_t SPI2_EN:1;  
    uint32_t SPI3_EN:1;  

    uint32_t :1;  
    uint32_t USART2_EN:1;  
    uint32_t USART3_EN:1; 
    uint32_t USART4_EN:1;  
    uint32_t USART5_EN:1; 
    uint32_t I2C1_EN:1;
    uint32_t I2C2_EN:1;
    uint32_t USBD_EN:1;

    uint32_t :1;
    uint32_t CAN1_EN:1;
    uint32_t CAN2_EN:1;
    uint32_t BKP_EN:1;
    uint32_t PWR_EN:1;
    uint32_t DAC_EN:1;
    uint32_t :2;
};ASSERT_REG_IS_32BIT(R32_RCC_APB1PCENR)


struct R32_RCC_BDCTLR{
    uint32_t LSEON:1;
    uint32_t LSERDY:1;
    uint32_t LSEBYP:1;
    uint32_t :5;
    uint32_t RTCSEL:2;
    uint32_t :5;
    uint32_t RTCEN:1;
    uint32_t BDRST:1;
    uint32_t :15;
};ASSERT_REG_IS_32BIT(R32_RCC_BDCTLR);

struct RCC_Def{
    volatile R32_RCC_CTLR           CTLR;
    volatile R32_RCC_CFGR0          CFGR0;
    volatile R32_RCC_INTR           INTR;
    volatile R32_RCC_APB2PRSTR      APB2PRSTR;
    volatile R32_RCC_APB1PRSTR      APB1PRSTR;
    volatile R32_RCC_AHBPCENR       AHBPCENR;
    volatile R32_RCC_APB2PCENR      APB2PCENR;
    volatile R32_RCC_APB1PCENR      APB1PCENR;

    volatile R32_RCC_RSTSCKR        RSTSCKR;
    volatile R32_RCC_AHBRSTR        AHBRSTR;

    constexpr void enable_hsi(const bool en){
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

    constexpr void enable_hse(const bool en){
        CTLR.HSION = en;
    }

    constexpr bool is_hse_ready(){
        return CTLR.HSIRDY;
    }

    constexpr void enable_hse_bypass(const bool en){
        CTLR.HSEBYP = en;
    }

    constexpr void enable_css(const bool en){
        CTLR.CSSON = en;
    }

    constexpr void enable_pll(const uint8_t pll_idx, const bool en){
        switch(pll_idx){
            case 1: CTLR.PLLON = en; return;
            case 2: CTLR.PLLON = en; return;
            case 3: CTLR.PLLON = en; return;
        }
    }

    constexpr bool is_pll_ready(const uint8_t pll_idx){
        switch(pll_idx){
            case 1: return CTLR.PLLRDY;
            case 2: return CTLR.PLL2RDY;
            case 3: return CTLR.PLL3RDY;
        }
    }

    //00 HSI
    //01 HSE
    //10 PLL
    //*11 RESERVED
    constexpr void set_sysclk_source(const uint8_t source){
        CFGR0.SW = source;
    }

    constexpr uint8_t get_sysclk_source(){
        return CFGR0.SWS;
    }

    // 0xxx：SYSCLK 不分频；
    // 1000：SYSCLK 2 分频；
    // 1001：SYSCLK 4 分频；
    // 1010：SYSCLK 8 分频；
    // 1011：SYSCLK 16 分频；
    // 1100：SYSCLK 64 分频；
    // 1101：SYSCLK 128 分频；
    // 1110：SYSCLK 256 分频；
    // 1111：SYSCLK 512 分频。

    constexpr void set_ahb_prescale(const int prescale){
        CFGR0.HPRE = [&]{
            switch(prescale){
                default:
                case 1:   return 0b0000; // SYSCLK 不分频
                case 2:   return 0b1000; // SYSCLK 2 分频
                case 4:   return 0b1001; // SYSCLK 4 分频
                case 8:   return 0b1010; // SYSCLK 8 分频
                case 16:  return 0b1011; // SYSCLK 16 分频
                case 64:  return 0b1100; // SYSCLK 64 分频
                case 128: return 0b1101; // SYSCLK 128 分频
                case 256: return 0b1110; // SYSCLK 256 分频
                case 512: return 0b1111; // SYSCLK 512 分频
            }
        }();
    }


    // 0xx：HCLK 不分频；
    // 100：HCLK 2 分频；
    // 101：HCLK 4 分频；
    // 110：HCLK 8 分频；
    // 111：HCLK 16 分频
    constexpr void set_apb1_prescale(const int prescale){
        CFGR0.PPRE1 = [&]{
            switch(prescale){
                default:
                case 1:   return 0b000; // SYSCLK 不分频
                case 2:   return 0b100; // SYSCLK 2 分频
                case 4:   return 0b101; // SYSCLK 4 分频
                case 8:   return 0b110; // SYSCLK 8 分频
                case 16:  return 0b111; // SYSCLK 16 分频
            }
        }();
    }

    // 0xx：HCLK 不分频；
    // 100：HCLK 2 分频；
    // 101：HCLK 4 分频；
    // 110：HCLK 8 分频；
    // 111：HCLK 16 分频
    constexpr void set_apb2_prescale(const int prescale){
        CFGR0.PPRE2 = [&]{
            switch(prescale){
                default:
                case 1:   return 0b000; // SYSCLK 不分频
                case 2:   return 0b100; // SYSCLK 2 分频
                case 4:   return 0b101; // SYSCLK 4 分频
                case 8:   return 0b110; // SYSCLK 8 分频
                case 16:  return 0b111; // SYSCLK 16 分频
            }
        }();
    }

    // 00：PCLK2 2 分频后作为 ADC 时钟；
    // 01：PCLK2 4 分频后作为 ADC 时钟；
    // 10：PCLK2 6 分频后作为 ADC 时钟；
    // 11：PCLK2 8 分频后作为 ADC 时钟。
    constexpr void set_adc_prescale(const int prescale){
        CFGR0.ADC_PRE = [&]{
            switch(prescale){
                default:
                case 2:   return 0b00; // SYSCLK 不分频
                case 4:   return 0b01; // SYSCLK 2 分频
                case 6:   return 0b10; // SYSCLK 4 分频
                case 8:   return 0b11; // SYSCLK 8 分频
            }
        }();
    }

    // CH32V30x_D8：
    // 1：HSE 不分频或 2 分频送入 PLL；
    // 0：HSI 不分频或 2 分频送入 PLL。
    // 对于 CH32F20x_D8C、CH32V30x_D8C：
    // 1：PREDIV1 输出送入 PLL；
    // 0：HSI 不分频或 2 分频送入 PLL。
    constexpr void set_pll_source(const uint8_t source){

    }

    // 对于 CH32F20x_D6、CH32F20x_D8、CH32F20x_D8W、
    // CH32V20x_D6、CH32V20x_D8、CH32V20x_D8W、
    // CH32V30x_D8：
    // 0000：PLL 2 倍频输出； 0001：PLL 3 倍频输出；
    // 0010：PLL 4 倍频输出； 0011：PLL 5 倍频输出；
    // 0100：PLL 6 倍频输出； 0101：PLL 7 倍频输出；
    // 0110：PLL 8 倍频输出； 0111：PLL 9 倍频输出；
    // 1000：PLL 10 倍频输出； 1001：PLL 11 倍频输出；
    // 1010：PLL 12 倍频输出； 1011：PLL 13 倍频输出；
    // 1100：PLL 14 倍频输出； 1101：PLL 15 倍频输出；
    // 1110：PLL 16 倍频输出； 1111：PLL 18 倍频输出。
    // 对于 CH32F20x_D8C、CH32V30x_D8C：
    // 0000：PLL 18 倍频输出； 0001：PLL 3 倍频输出；
    // 0010：PLL 4 倍频输出； 0011：PLL 5 倍频输出；
    // 0100：PLL 6 倍频输出； 0101：PLL 7 倍频输出；
    // 0110：PLL 8 倍频输出； 0111：PLL 9 倍频输出；
    // 1000：PLL 10 倍频输出； 1001：PLL 11 倍频输出；
    constexpr void set_pll_multiplier(const uint8_t multiplier){

    }

    // USBFS/USBOTG 时钟分频配置：
    // 00：1 分频（适用于 PLLCLK=48MHz）；
    // 01：2 分频（适用于 PLLCLK=96MHz）；
    // 10：3 分频（适用于 PLLCLK=144MHz）；
    // 11：5 分频，且 PLL 的源为 HSE 二分频（适用于
    // PLLCLK=240MHz ， 仅 适 用 于 CH32V20x_D8W/ 
    // CH32F20x_D8W）。
    // 注：CH32V20x_D8W、CH32F20x_D8W 具有 11b 选项，其
    // 余型号该选项保留。
    // USBD/USBHD 模块时钟需要 48MHz。此位必须在 USBD 和
    // USBHD 时 钟 使 能 前 进 行 配 置 （ RCC_AHBPCENR 和
    // RCC_APB1PCENR 中）

    constexpr void set_usb_prescale(const int prescale){

    }

    // 微控制器 MCO 引脚时钟输出控制：
    // 00xx：没有时钟输出；
    // 0100：系统时钟(SYSCLK)输出；
    // 0101：内部 8MHz 的 RC 振荡器时钟(HSI)输出；
    // 0110：外部振荡器时钟(HSE)输出；
    // 0111：PLL 时钟 2 分频后输出；
    // 1000：PLL2 时钟输出；
    // 1001：PLL3 时钟 2 分频后输出；
    // 1010：XT1 外部震荡器时钟输出；
    // 1011：PLL3 时钟输出。
    // 注：在启动或切换 MCO 时钟时，可能有几个周期的时
    // 钟 丢 失 。 其 中 1000 — — 1011 适 用 于 适 用 于
    // CH32F20x_D8C、CH32V30x_D8C。
    constexpr void set_mco_source(const uint8_t source){
        CFGR0.MCO = source;
    }

    // 以太网时钟来源预分频控制：
    // 0：不分频；
    // 1：2分频；
    constexpr void set_eth_prescale(const int prescale){
        CFGR0.ETH_PRE = [&]{
            switch(prescale){
                default:
                case 1:   return 0b00; // SYSCLK 不分频
                case 2:   return 0b01; // SYSCLK 2 分频
            }
        }();
    }

    // ADC 时钟占空比调整：
    // 1：ADC 时钟低电平时间更长；
    // 0：ADC 时钟占空比为 50%。
    constexpr void set_adc_duty(const bool duty){
        CFGR0.ADC_DUTY = duty;
    }

    constexpr void reset_i2c(const uint8_t index){
        switch(index){
            default:    return;
            case 1:   APB1PRSTR.I2C1_RST = 1;return;
            case 2:   APB1PRSTR.I2C2_RST = 1;return;
        }
    }

    constexpr void reset_timer(const uint8_t index){
        switch(index){
            default:    return;
            case 1:   APB2PRSTR.TIM1_RST = 1; return;
            case 2:   APB1PRSTR.TIM2_RST = 1; return;
            case 3:   APB1PRSTR.TIM3_RST = 1; return;
            case 4:   APB1PRSTR.TIM4_RST = 1; return;
            case 5:   APB1PRSTR.TIM5_RST = 1; return;
            case 6:   APB1PRSTR.TIM6_RST = 1; return;
            case 7:   APB1PRSTR.TIM7_RST = 1; return;
            case 8:   APB2PRSTR.TIM8_RST = 1; return;
            case 9:   APB2PRSTR.TIM9_RST = 1; return;
            case 10:  APB2PRSTR.TIM10_RST = 1; return;
        }
    }

    constexpr void reset_usart(const uint8_t index){
        switch(index){
            default:    return;
            case 1:   APB2PRSTR.USART1_RST = 1; return;
            case 2:   APB1PRSTR.USART2_RST = 1; return;
            case 3:   APB1PRSTR.USART3_RST = 1; return;
            case 4:   APB1PRSTR.USART4_RST = 1; return;
            case 5:   APB1PRSTR.USART5_RST = 1; return;
            case 6:   APB1PRSTR.USART6_RST = 1; return;
            case 7:   APB1PRSTR.USART7_RST = 1; return;
            case 8:   APB1PRSTR.USART8_RST = 1; return;
        }
    }

    constexpr void reset_spi(const uint8_t index){
        switch(index){
            default:    return;
            case 1:   APB2PCENR.SPI1_EN = 1; return;
            case 2:   APB1PCENR.SPI2_EN = 1; return;
            case 3:   APB1PCENR.SPI3_EN = 1; return;
        }
    }

    constexpr void enable_i2c(const uint8_t index){
        switch(index){
            default:    return;
            case 1:   APB1PCENR.I2C1_EN = 1;return;
            case 2:   APB1PCENR.I2C2_EN = 1;return;
        }
    }

    constexpr void enable_timer(const uint8_t index){
        switch(index){
            default:    return;
            case 1:   APB2PCENR.TIM1_EN = 1; return;
            case 2:   APB1PCENR.TIM2_EN = 1; return;
            case 3:   APB1PCENR.TIM3_EN = 1; return;
            case 4:   APB1PCENR.TIM4_EN = 1; return;
            case 5:   APB1PCENR.TIM5_EN = 1; return;
            case 6:   APB1PCENR.TIM6_EN = 1; return;
            case 7:   APB1PCENR.TIM7_EN = 1; return;
            case 8:   APB2PCENR.TIM8_EN = 1; return;
            case 9:   APB2PCENR.TIM9_EN = 1; return;
            case 10:  APB2PCENR.TIM10_EN = 1; return;
        }
    }

    constexpr void enable_usart(const uint8_t index){
        switch(index){
            default:    return;
            case 1:   APB2PCENR.USART1_EN = 1; return;
            case 2:   APB1PCENR.USART2_EN = 1; return;
            case 3:   APB1PCENR.USART3_EN = 1; return;
            case 4:   APB1PCENR.USART4_EN = 1; return;
            case 5:   APB1PCENR.USART5_EN = 1; return;
            case 6:   APB1PCENR.USART6_EN = 1; return;
            case 7:   APB1PCENR.USART7_EN = 1; return;
            case 8:   APB1PCENR.USART8_EN = 1; return;
        }
    }

    constexpr void enable_spi(const uint8_t index){
        switch(index){
            default:    return;
            case 1:   APB2PCENR.SPI1_EN = 1; return;
            case 2:   APB1PCENR.SPI2_EN = 1; return;
            case 3:   APB1PCENR.SPI3_EN = 1; return;
        }
    }
};

}