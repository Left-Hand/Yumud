#pragma once

#include <cstdint>

#ifndef ASSERT_REG_IS_32BIT
#define ASSERT_REG_IS_32BIT(name) static_assert((sizeof(name) == 4),  #name " is not 32bit"); 
#endif


namespace CH32V20x{

struct R32_AFIO_ECR{
    uint32_t PIN:4;
    uint32_t PORT:3;
    uint32_t EVOE:1;
    uint32_t :24;
};

struct R32_AFIO_PCFR1{
    uint32_t SPI1_RM:1;
    uint32_t I2C1_RM:1;
    uint32_t USART1_RM:1;
    uint32_t USART2_RM:1;
    uint32_t USART3_RM:2;
    uint32_t TIM1_RM:2;

    uint32_t TIM2_RM:2;
    uint32_t TIM3_RM:2;
    uint32_t TIM4_RM:1;
    uint32_t CAN_RM:2;
    uint32_t PDO1_RM:1;

    uint32_t TIM5CH4_RM:1;
    uint32_t ADC1_ETRGINJ:1;
    uint32_t ADC1_ETRGREG:1;
    uint32_t ADC2_ETRGINJ:1;
    uint32_t ADC2_ETRGREG:1;
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

// 

struct R32_AFIO_EXTICR1{
    uint32_t EXTI0:4;
    uint32_t EXTI1:4;
    uint32_t EXTI2:4;
    uint32_t EXTI3:4;
    uint32_t :16;
};

struct R32_AFIO_EXTICR2{
    uint32_t EXTI4:4;
    uint32_t EXTI5:4;
    uint32_t EXTI6:4;
    uint32_t EXTI7:4;
    uint32_t :16;
};

struct R32_AFIO_EXTICR3{
    uint32_t EXTI8:4;
    uint32_t EXTI9:4;
    uint32_t EXTI10:4;
    uint32_t EXTI11:4;
    uint32_t :16;
};

struct R32_AFIO_EXTICR4{
    uint32_t EXTI12:4;
    uint32_t EXTI13:4;
    uint32_t EXTI14:4;
    uint32_t EXTI15:4;
    uint32_t :16;
};

struct R32_AFIO_PCFR2{
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
    uint32_t USART1_RM1:1;
    uint32_t :5;
};

// struct R32_AFIO_PCFG2{
    //     uint32_t TIM8_RM:2;
    //     uint32_t USART6_RM:2;
    //     uint32_t USART7_RM:2;
    //     uint32_t USART8_RM:2;
    //     uint32_t TIM9_RM:2;
    //     uint32_t TIM10_RM:2;
    //     uint32_t TIM11_RM:2;
    // };
    

struct AFIO_Def{

    volatile R32_AFIO_ECR       ECR;
    volatile R32_AFIO_PCFR1     PCFR1;
    volatile R32_AFIO_EXTICR1   EXTICR1;
    volatile R32_AFIO_EXTICR2   EXTICR2;
    volatile R32_AFIO_EXTICR3   EXTICR3;
    volatile R32_AFIO_EXTICR4   EXTICR4;
    volatile R32_AFIO_PCFR2     PCFR2;
    
    ASSERT_REG_IS_32BIT(R32_AFIO_ECR)
    ASSERT_REG_IS_32BIT(R32_AFIO_PCFR1)
    ASSERT_REG_IS_32BIT(R32_AFIO_EXTICR1)
    ASSERT_REG_IS_32BIT(R32_AFIO_EXTICR2)
    ASSERT_REG_IS_32BIT(R32_AFIO_EXTICR3)
    ASSERT_REG_IS_32BIT(R32_AFIO_EXTICR4)
    ASSERT_REG_IS_32BIT(R32_AFIO_PCFR2)


    void set_eventout(const uint8_t pin, const uint8_t port, const bool en){
        ECR.EVOE = en;
        ECR.PIN = pin;
        ECR.PORT = port;
    }

    void set_spi1_remap(const uint8_t remap){
        PCFR1.SPI1_RM = remap & 0x01;
    }

    void set_i2c1_remap(const uint8_t remap){
        PCFR1.I2C1_RM = remap & 0x01;
    }

    void set_usart1_remap(const uint8_t remap){
        PCFR1.USART1_RM = remap & 0x01;
    }

    void set_usart2_remap(const uint8_t remap){
        PCFR1.USART2_RM = remap & 0x01;
    }

    void set_usart3_remap(const uint8_t remap){
        PCFR1.USART3_RM = remap & 0x03;
    }

    void set_tim1_remap(const uint8_t remap){
        PCFR1.TIM1_RM = remap & 0x03;
    }

    void set_tim2_remap(const uint8_t remap){
        PCFR1.TIM2_RM = remap & 0x03;
    }

    void set_tim3_remap(const uint8_t remap){
        PCFR1.TIM3_RM = remap & 0x03;
    }

    void set_tim4_remap(const uint8_t remap){
        PCFR1.TIM4_RM = remap & 0x01;
    }

    void set_can1_remap(const uint8_t remap){
        PCFR1.CAN_RM = remap & 0x03;
    }

    void set_pdo1_remap(const uint8_t remap){
        PCFR1.PDO1_RM = remap & 0x01;
    }

    void set_tim5ch4_remap(const uint8_t remap){
        PCFR1.TIM5CH4_RM = remap & 0x01;
    }

    //为1时与TIM8_CH4连接
    void set_adc1_etrig_inj_remap(const uint8_t remap){
        PCFR1.ADC1_ETRGINJ = remap & 0x01;
    }
    
    //为1时与TIM8_TRGO连接
    void set_adc1_etrig_reg_remap(const uint8_t remap){
        PCFR1.ADC1_ETRGREG = remap & 0x01;
    }

    //为1时与TIM8_CH4连接
    void set_adc2_etrig_inj_remap(const uint8_t remap){
        PCFR1.ADC2_ETRGINJ = remap & 0x01;
    }

    //为1时与TIM8_TRGO连接
    void set_adc2_etrig_reg_remap(const uint8_t remap){
        PCFR1.ADC2_ETRGREG = remap & 0x01;
    }

    void set_eth_remap(const uint8_t remap){
        PCFR1.ETH_RM = remap & 0x01;
    }

    void set_can2_remap(const uint8_t remap){
        PCFR1.CAN2_RM = remap & 0x01;
    }

    void set_miirmii_sel(const uint8_t remap){
        PCFR1.MIIRMII_SEL = remap & 0x01;
    }

    void set_sw_cfg(const uint8_t remap){
        PCFR1.SW_CFG = remap & 0x07;
    }

    void set_spi3_remap(const uint8_t remap){
        PCFR1.SPI3_RM = remap & 0x01;
    }

    void set_tim2_itr1_remap(const uint8_t remap){
        PCFR1.TIM2_ITR1_RM = remap & 0x01;
    }

    void set_ptp_pps_remap(const uint8_t remap){
        PCFR1.PTP_PPS_RM = remap & 0x01;
    }
    
    void set_tim8_remap(const uint8_t remap){
        PCFR2.TIM8_RM = remap & 0x01;
    }

    void set_tim9_remap(const uint8_t remap){
        PCFR2.TIM9_RM = remap & 0x03;
    }

    void set_tim10_remap(const uint8_t remap){
        PCFR2.TIM10_RM = remap & 0x03;
    }

    void set_fsmc_nadv_remap(const uint8_t remap){
        PCFR2.FSMC_NADV = remap & 0x01;
    }

    void set_usart4_remap(const uint8_t remap){
        PCFR2.USART4_RM = remap & 0x03;
    }

    void set_usart5_remap(const uint8_t remap){
        PCFR2.USART5_RM = remap & 0x03;
    }

    void set_usart6_remap(const uint8_t remap){
        PCFR2.USART6_RM = remap & 0x03;
    }

    void set_usart7_remap(const uint8_t remap){
        PCFR2.USART7_RM = remap & 0x03;
    }

    void set_usart8_remap(const uint8_t remap){
        PCFR2.USART8_RM = remap & 0x03;
    }

    void set_usart1_rm1_remap(const uint8_t remap){
        PCFR2.USART1_RM1 = remap & 0x01;
    }

    void set_exti_source(const uint8_t port, const uint8_t pin){
        switch(pin & 0x0f){
            case 0: EXTICR1.EXTI0 = port; return;
            case 1: EXTICR1.EXTI1 = port; return;
            case 2: EXTICR1.EXTI2 = port; return;
            case 3: EXTICR1.EXTI3 = port; return;
    
            case 4: EXTICR2.EXTI4 = port; return;
            case 5: EXTICR2.EXTI5 = port; return;
            case 6: EXTICR2.EXTI6 = port; return;
            case 7: EXTICR2.EXTI7 = port; return;
    
            case 8: EXTICR3.EXTI8 = port; return;
            case 9: EXTICR3.EXTI9 = port; return;
            case 10: EXTICR3.EXTI10 = port; return;
            case 11: EXTICR3.EXTI11 = port; return;
    
            case 12: EXTICR4.EXTI12 = port; return;
            case 13: EXTICR4.EXTI13 = port; return;
            case 14: EXTICR4.EXTI14 = port; return;
            case 15: EXTICR4.EXTI15 = port; return;
        }
    }

    void set_tim_remap(const uint8_t index, const uint8_t remap){
        switch(index){
            default: return;
            case 1: set_tim1_remap(remap); return;
            case 2: set_tim2_remap(remap); return;
            case 3: set_tim3_remap(remap); return;
            case 4: set_tim4_remap(remap); return;
            case 5: set_tim5ch4_remap(remap); return;
            case 8: set_tim8_remap(remap); return;
            case 9: set_tim9_remap(remap); return;
            case 10: set_tim10_remap(remap); return;
        }
    }

    void set_usart_remap(const uint8_t index, const uint8_t remap){
        switch(index){
            default: return;
            case 1: set_usart1_remap(remap);
            case 2: set_usart2_remap(remap);
            case 3: set_usart3_remap(remap);
            case 4: set_usart4_remap(remap);
            case 5: set_usart5_remap(remap);
            case 6: set_usart6_remap(remap);
            case 7: set_usart7_remap(remap);
            case 8: set_usart8_remap(remap);
        }
    }

    void set_spi_remap(const uint8_t index, const uint8_t remap){
        switch(index){
            default: return;
            case 1: set_spi1_remap(remap); return;
            case 3: set_spi3_remap(remap); return;
        }
    }

    void set_i2c_remap(const uint8_t index, const uint8_t remap){
        switch(index){
            default: return;
            case 1: set_i2c1_remap(remap); return;
        }
    }

    void set_can_remap(const uint8_t index, const uint8_t remap){
        switch(index){
            default: return;
            case 1: set_can1_remap(remap); return;
            case 2: set_can2_remap(remap); return;
        }
    }
};



static inline AFIO_Def * AFIO_Inst = (AFIO_Def *)(0x40010000);



}