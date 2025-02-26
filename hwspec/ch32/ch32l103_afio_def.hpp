#pragma once

#include <cstdint>

namespace hwspec::CH32V00x::AFIO_Regs{

    
struct R32_AFIO_ECR{
    uint32_t PIN:4;
    uint32_t PORT:3;
    uint32_t EVOE:1;
    uint32_t :24;
};

struct R32_AFIO_PCFR1{
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

struct R32_AFIO_CR{
    uint32_t :8;
    uint32_t USBPD_IN_HVT:2;
    uint32_t :6;

    uint32_t UDM_BC_VSRC:1;
    uint32_t UDP_BC_VSRC:1;
    uint32_t UDM_BC_CMPE:1;
    uint32_t UDM_BC_CMPE:1;
    uint32_t UDP_BC_CMPO:1;
    uint32_t UDP_BC_CMPO:1;
    uint32_t :10;
};

struct R32_AFIO_PCFR2{
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

struct AFIO_Def{
    volatile R32_AFIO_ECR        ECR;
    volatile R32_AFIO_PCFR1      PCFR1;
    volatile R32_AFIO_EXTICR1    EXTICR1;
    volatile R32_AFIO_EXTICR2    EXTICR2;
    volatile R32_AFIO_EXTICR3    EXTICR3;
    volatile R32_AFIO_EXTICR4    EXTICR4;
    volatile R32_AFIO_CR         CR;
    volatile R32_AFIO_PCFR2      PCFR2;

    void SET_SWCFG(const uint8_t val){
        PCFR1.SWCFG = val;
    }

    void SET_USART3_RM(const uint8_t val){
        PCFR1.USART3_RM = val;
    }

    void SET_TIM3_RM(const uint8_t val){
        PCFR1.TIM3_RM = val;
    }
};
}