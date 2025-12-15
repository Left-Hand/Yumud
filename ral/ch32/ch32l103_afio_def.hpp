#pragma once

#include <cstdint>

namespace ymd::ral::CH32V00x::AFIO_Regs{

    
struct [[nodiscard]] R32_AFIO_ECR{
    uint32_t PIN:4;
    uint32_t PORT:3;
    uint32_t EVOE:1;
    uint32_t :24;
};

struct [[nodiscard]] R32_AFIO_PCFR1{
    // 00：默认映射（NSS/PA4，SCK/PA5，MISO/PA6，
    // MOSI/PA7）
    // 01：完全映射（NSS/PA15，SCK/PB3，MISO/PB4，
    // MOSI/PB5）
    // 10：完全映射（NSS/PA12，SCK/PB6，MISO/PB8，
    // MOSI/PB7）
    // 11：完全映射（NSS/PB12，SCK/PB6，MISO/PB8，
    // MOSI/PB7）
    uint32_t SPI1_RM:1;

    // 00：默认映射（SCL/PB6，SDA/PB7）
    // 10：完全映射（SCL/PA13，SDA/PA12）
    // 11：完全映射（SCL/PB9，SDA/PB11）
    // 0 
    // 0 SPI1_RM RW 
    // SPI1 的 重 映 射 ， 和 重 映 射 寄 存 器
    uint32_t I2C1_RM:1;
    uint32_t USART1_RM:1;
    uint32_t USART2_RM:1;
    uint32_t USART3_RM:2;
    uint32_t TIM1_RM:2;

    uint32_t TIM2_RM:2;
    uint32_t TIM3_RM:1;
    uint32_t :1;
    uint32_t TIM4_RM:1;
    uint32_t CAN_RM:2;
    uint32_t PD0PD1_RM:1;

    uint32_t :8;
    uint32_t SWCFG:3;
    uint32_t :5;
};

struct [[nodiscard]] R32_AFIO_EXTICR1{
    uint32_t EXTI0:4;
    uint32_t EXTI1:4;
    uint32_t EXTI2:4;
    uint32_t EXTI3:4;
    uint32_t :16;
};

struct [[nodiscard]] R32_AFIO_EXTICR2{
    uint32_t EXTI4:4;
    uint32_t EXTI5:4;
    uint32_t EXTI6:4;
    uint32_t EXTI7:4;
    uint32_t :16;
};

struct [[nodiscard]] R32_AFIO_EXTICR3{
    uint32_t EXTI8:4;
    uint32_t EXTI9:4;
    uint32_t EXTI10:4;
    uint32_t EXTI11:4;
    uint32_t :16;
};

struct [[nodiscard]] R32_AFIO_EXTICR4{
    uint32_t EXTI12:4;
    uint32_t EXTI13:4;
    uint32_t EXTI14:4;
    uint32_t EXTI15:4;
    uint32_t :16;
};

struct [[nodiscard]] R32_AFIO_CR{
    uint32_t :8;
    uint32_t USBPD_IN_HVT:2;
    uint32_t :6;

    uint32_t UDP_BC_VSRC:1;
    uint32_t UDM_BC_VSRC:1;
    uint32_t UDP_BC_CMPE:1;
    uint32_t UDM_BC_CMPE:1;
    uint32_t UDP_BC_CMPO:1;
    uint32_t UDM_BC_CMPO:1;
    uint32_t :10;
};

struct [[nodiscard]] R32_AFIO_PCFR2{
    uint32_t :16;
    uint32_t USART4_RM:1;
    uint32_t :1;
    uint32_t USART2_RM_H:1;
    uint32_t USART1_RM_H:2;
    uint32_t TIM2_RM_H:2;
    uint32_t TIM1_RM_H:2;

    uint32_t I2C1_RM_H:1;
    uint32_t SPI1_RM_H:1;
    uint32_t TIM4_RM:1;
    uint32_t LPTIM_RM:1;

    uint32_t :6;
};

struct [[nodiscard]] AFIO_Def{
    volatile R32_AFIO_ECR        ECR;
    volatile R32_AFIO_PCFR1      PCFR1;
    volatile R32_AFIO_EXTICR1    EXTICR1;
    volatile R32_AFIO_EXTICR2    EXTICR2;
    volatile R32_AFIO_EXTICR3    EXTICR3;
    volatile R32_AFIO_EXTICR4    EXTICR4;
    volatile R32_AFIO_CR         CR;
    volatile R32_AFIO_PCFR2      PCFR2;

    constexpr void set_spi1_remap(const uint8_t remap){
        PCFR1.SPI1_RM = remap & 0b1;
        PCFR2.SPI1_RM_H = remap >> 1;
    }

    constexpr void set_i2c1_remap(const uint8_t remap){
        PCFR1.I2C1_RM = remap & 0b1;
        PCFR2.I2C1_RM_H = remap >> 1;
    }

    constexpr void set_usart1_remap(const uint8_t remap){
        PCFR1.USART1_RM = remap & 0b1;
        PCFR2.USART1_RM_H = remap >> 1;
    }

    constexpr void set_usart2_remap(const uint8_t remap){
        PCFR1.USART2_RM = remap & 0b1;
        PCFR2.USART2_RM_H = remap >> 1;
    }

    constexpr void set_usart3_remap(const uint8_t remap){
        PCFR1.USART3_RM = remap & 0b11;
    }

    constexpr void set_tim1_remap(const uint8_t remap){
        PCFR1.TIM1_RM = remap & 0b11;
        PCFR2.TIM1_RM_H = remap >> 2;
    }

    constexpr void set_tim2_remap(const uint8_t remap){
        PCFR1.TIM2_RM = remap & 0b11;
        PCFR2.TIM2_RM_H = remap >> 2;
    }

    constexpr void set_tim3_remap(const uint8_t remap){
        PCFR1.TIM3_RM = remap & 0b1;
    }

    constexpr void set_tim4_remap(const uint8_t remap){
        PCFR1.TIM4_RM = remap & 0b1;
    }

    constexpr void set_can1_remap(const uint8_t remap){
        PCFR1.CAN_RM = remap & 0b11;
    }

    constexpr void set_pd0pd1_remap(const uint8_t remap){
        PCFR1.PD0PD1_RM = remap & 0b1;
    }

    // unchecked
    // constexpr void set_swcfg_remap(const uint8_t remap){
    //     PCFR1.SWCFG = remap & 0b111;
    // }

    constexpr void enable_usbpd_high_threshold(const Enable en){
        CR.USBPD_IN_HVT = en;
    }

    constexpr void enable_udp_bc_source_volt(const Enable en){
        CR.UDP_BC_VSRC = en;
    }

    constexpr void enable_udm_bc_source_volt(const Enable en){
        CR.UDM_BC_VSRC = en;
    }

    constexpr void enable_udp_bc_cmp(const Enable en){
        CR.UDP_BC_CMPE = en;
    }

    constexpr void enable_udm_bc_cmp(const Enable en){
        CR.UDM_BC_CMPE = en;
    }

    constexpr bool get_udp_bc_cmp_state(){
        return CR.UDP_BC_CMPO;
    }

    constexpr bool get_udm_bc_cmp_state(){
        return CR.UDM_BC_CMPO;
    }

    constexpr void set_usart4_remap(const uint8_t remap){
        PCFR2.USART4_RM = remap & 0b1;
    }

    constexpr void set_lptim_remap(const uint8_t remap){
        PCFR2.LPTIM_RM = remap & 0b1;
    }

    constexpr void set_exti_source(const uint8_t port_source, const uint8_t pin_source){
        switch(pin_source & 0x0f){
            case 0: EXTICR1.EXTI0 = port_source; return;
            case 1: EXTICR1.EXTI1 = port_source; return;
            case 2: EXTICR1.EXTI2 = port_source; return;
            case 3: EXTICR1.EXTI3 = port_source; return;
    
            case 4: EXTICR2.EXTI4 = port_source; return;
            case 5: EXTICR2.EXTI5 = port_source; return;
            case 6: EXTICR2.EXTI6 = port_source; return;
            case 7: EXTICR2.EXTI7 = port_source; return;
    
            case 8: EXTICR3.EXTI8 = port_source; return;
            case 9: EXTICR3.EXTI9 = port_source; return;
            case 10: EXTICR3.EXTI10 = port_source; return;
            case 11: EXTICR3.EXTI11 = port_source; return;
    
            case 12: EXTICR4.EXTI12 = port_source; return;
            case 13: EXTICR4.EXTI13 = port_source; return;
            case 14: EXTICR4.EXTI14 = port_source; return;
            case 15: EXTICR4.EXTI15 = port_source; return;
        }
    }

    void set_tim_remap(const uint8_t index, const uint8_t remap){
        switch(index){
            default: return;
            case 1: set_tim1_remap(remap); return;
            case 2: set_tim2_remap(remap); return;
            case 3: set_tim3_remap(remap); return;
            case 4: set_tim4_remap(remap); return;
        }
    }

    void set_usart_remap(const uint8_t index, const uint8_t remap){
        switch(index){
            default: return;
            case 1: set_usart1_remap(remap);
            case 2: set_usart2_remap(remap);
            case 3: set_usart3_remap(remap);
            case 4: set_usart4_remap(remap);
        }
    }

    void set_spi_remap(const uint8_t index, const uint8_t remap){
        switch(index){
            default: return;
            case 1: set_spi1_remap(remap); return;
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
        }
    }
};
}