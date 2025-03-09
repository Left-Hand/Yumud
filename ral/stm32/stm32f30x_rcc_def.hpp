#pragma once

#include <cstdint>

namespace hwspec{
    namespace STM32F30x::RCC_Def{
        typedef struct
        {
        volatile uint32_t CR;         /*!< RCC clock control register,                                  Address offset: 0x00 */
        volatile uint32_t CFGR;       /*!< RCC clock configuration register,                            Address offset: 0x04 */
        volatile uint32_t CIR;        /*!< RCC clock interrupt register,                                Address offset: 0x08 */
        volatile uint32_t APB2RSTR;   /*!< RCC APB2 peripheral reset register,                          Address offset: 0x0C */
        volatile uint32_t APB1RSTR;   /*!< RCC APB1 peripheral reset register,                          Address offset: 0x10 */
        volatile uint32_t AHBENR;     /*!< RCC AHB peripheral clock register,                           Address offset: 0x14 */
        volatile uint32_t APB2ENR;    /*!< RCC APB2 peripheral clock enable register,                   Address offset: 0x18 */
        volatile uint32_t APB1ENR;    /*!< RCC APB1 peripheral clock enable register,                   Address offset: 0x1C */
        volatile uint32_t BDCR;       /*!< RCC Backup domain control register,                          Address offset: 0x20 */
        volatile uint32_t CSR;        /*!< RCC clock control & status register,                         Address offset: 0x24 */
        volatile uint32_t AHBRSTR;    /*!< RCC AHB peripheral reset register,                           Address offset: 0x28 */
        volatile uint32_t CFGR2;      /*!< RCC clock configuration register 2,                          Address offset: 0x2C */
        volatile uint32_t CFGR3;      /*!< RCC clock configuration register 3,                          Address offset: 0x30 */
        } RCC_TypeDef;
    }
}