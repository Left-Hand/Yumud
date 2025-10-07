#pragma once

#include <cstdint>
#include "ral/chip.hpp"

namespace ymd::hal::rcc{
enum class APB1Periph:uint32_t{
    _TIM2              = 0x00000001u,
    _TIM3              = 0x00000002u,
    _TIM4              = 0x00000004u,
    _TIM5              = 0x00000008u,
    _TIM6              = 0x00000010u,
    _TIM7              = 0x00000020u,
    _UART6             = 0x00000040u,
    _UART7             = 0x00000080u,
    _UART8             = 0x00000100u,
    _WWDG              = 0x00000800u,
    _SPI2              = 0x00004000u,
    _SPI3              = 0x00008000u,
    _USART2            = 0x00020000u,
    _USART3            = 0x00040000u,
    _UART4             = 0x00080000u,
    _UART5             = 0x00100000u,
    _I2C1              = 0x00200000u,
    _I2C2              = 0x00400000u,
    _USB               = 0x00800000u,
    _CAN1              = 0x02000000u,
    _CAN2              = 0x04000000u,
    _BKP               = 0x08000000u,
    _PWR               = 0x10000000u,
    _DAC               = 0x20000000u,
};


enum class APB2Periph:uint32_t{
    _AFIO              = 0x00000001u,
    _GPIOA             = 0x00000004u,
    _GPIOB             = 0x00000008u,
    _GPIOC             = 0x00000010u,
    _GPIOD             = 0x00000020u,
    _GPIOE             = 0x00000040u,
    _ADC1              = 0x00000200u,
    _ADC2              = 0x00000400u,
    _TIM1              = 0x00000800u,
    _SPI1              = 0x00001000u,
    _TIM8              = 0x00002000u,
    _USART1            = 0x00004000u,
    _TIM9              = 0x00080000u,
    _TIM10             = 0x00100000u,
};

enum class AHBPeriph:uint32_t{
    _DMA1               = 0x00000001u,
    _DMA2               = 0x00000002u,
    _SRAM               = 0x00000004u,
    _CRC                = 0x00000040u,
    _FSMC               = 0x00000100u,
    _RNG                = 0x00000200u,
    _SDIO               = 0x00000400u,
    _USBHS              = 0x00000800u,
    _OTG_FS             = 0x00001000u,
    _DVP                = 0x00002000u,
    _ETH_MAC            = 0x00004000u,
    _ETH_MAC_Tx         = 0x00008000u,
    _ETH_MAC_Rx         = 0x00010000u,
};

}