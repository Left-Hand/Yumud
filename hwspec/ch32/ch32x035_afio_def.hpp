#pragma once

#include <cstdint>

namespace hwspec::CH32V20x::AFIO_Regs{

struct PCFR1_Reg{
    // SPI1 的重映射。该位可由用户读写。它控制 SPI1
    // 的 NSS、CK、MISO 和 MOSI 复用功能在 GPIO 端口
    // 的映射：
    // 00：映射（NSS/PA4，CK/PA5，MISO/PA6，MOSI/PA7）
    // 01：映射（NSS/PB21，CK/PB15，MISO/PA8，
    // MOSI/PA9）
    // 10：映射（NSS/PA12，CK/PA11，MISO/PA9，
    // MOSI/PA10）
    // 11：映射（NSS/PC4，CK/PC5，MISO/PC6，MOSI/PC7）
    uint32_t SPI1_RM:2;

    // I2C1 的重映射。该位可由用户读写。它控制 I2C1
    // 的 SCL 和 SDA 复用功能在 GPIO 端口的映射：
    // 000：映射（SCL/PA10，SDA/PA11）
    // 001：映射（SCL/PA13，SDA/PA14）
    // 010：映射（SCL/PC16，SDA/PC17）
    // 011：映射（SCL/PC19，SDA/PC18）
    // 1x0：映射（SCL/PC17，SDA/PC16）
    // 1x1：映射（SCL/PC18，SDA/PC19）
    uint32_t I2C1_RM:2;

    uint32_t USART1_RM:2;
    uint32_t USART2_RM:2;
    uint32_t USART3_RM:2;
    uint32_t USART4_RM:2;

    uint32_t TIM1_RM:2;
    uint32_t TIM2_RM:2;
    uint32_t TIM3_RM:2;
    uint32_t PIOC_RM:2;
    uint32_t SW_CFG:2;
    uint32_t :5;
};

struct EXTICR1_Reg{
    uint32_t EXTI0:2;
    uint32_t EXTI1:2;
    uint32_t EXTI2:2;
    uint32_t EXTI3:2;
    
    uint32_t EXTI4:2;
    uint32_t EXTI5:2;
    uint32_t EXTI6:2;
    uint32_t EXTI7:2;

    uint32_t EXTI8:2;
    uint32_t EXTI9:2;
    uint32_t EXTI10:2;
    uint32_t EXTI11:2;
    
    uint32_t EXTI12:2;
    uint32_t EXTI13:2;
    uint32_t EXTI14:2;
    uint32_t EXTI15:2;    
};

struct EXTICR2_Reg{
    uint32_t EXTI16:2;
    uint32_t EXTI17:2;
    
    uint32_t EXTI18:2;
    uint32_t EXTI19:2;
    uint32_t EXTI20:2;
    uint32_t EXTI21:2;  
    uint32_t EXTI22:2;  
    uint32_t EXTI23:2;  

    uint32_t :16;
};

struct CTLR_Reg{
    uint32_t UDM_PUE:2;
    uint32_t UDP_PUE:2;
    uint32_t :2;
    uint32_t USB_PHY_V33:1;
    uint32_t USB_IOEN:1;
    uint32_t USBPG_PHY_V33:1;
    uint32_t USBPD_IN_HVT:1;
    uint32_t :6;
    uint32_t UDP_BC_VSRC:1;
    uint32_t UDM_BC_VSRC:1;
    uint32_t UDP_BC_CMPO:1;
    uint32_t UDM_BC_CMPO:1;
    uint32_t :3;
    uint32_t PA3_FILT_EN:1;
    uint32_t PA4_FILT_EN:1;
    uint32_t PA5_FILT_EN:1;
    uint32_t PA6_FILT_EN:1;
    uint32_t :4;
};

struct AFIO_Def{
    volatile PCFR1_Reg PCFR1;
    volatile EXTICR1_Reg EXTICR1;
    volatile EXTICR2_Reg EXTICR2;
    volatile CTLR_Reg CTLR;
};

}