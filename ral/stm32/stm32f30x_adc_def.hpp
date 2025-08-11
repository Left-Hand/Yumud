#pragma once

#include <cstdint>

#define __IO volatile


namespace ymd::ral::STM32F30x::ADC{

    typedef struct
    {
    __IO uint32_t ISR;              /*!< ADC Interrupt and Status Register,                 Address offset: 0x00 */
    __IO uint32_t IER;              /*!< ADC Interrupt Enable Register,                     Address offset: 0x04 */
    __IO uint32_t CR;               /*!< ADC control register,                              Address offset: 0x08 */
    __IO uint32_t CFGR;             /*!< ADC Configuration register,                        Address offset: 0x0C */
    uint32_t      RESERVED0;        /*!< Reserved, 0x010                                                         */
    __IO uint32_t SMPR1;            /*!< ADC sample time register 1,                        Address offset: 0x14 */
    __IO uint32_t SMPR2;            /*!< ADC sample time register 2,                        Address offset: 0x18 */
    uint32_t      RESERVED1;        /*!< Reserved, 0x01C                                                         */
    __IO uint32_t TR1;              /*!< ADC watchdog threshold register 1,                 Address offset: 0x20 */
    __IO uint32_t TR2;              /*!< ADC watchdog threshold register 2,                 Address offset: 0x24 */
    __IO uint32_t TR3;              /*!< ADC watchdog threshold register 3,                 Address offset: 0x28 */
    uint32_t      RESERVED2;        /*!< Reserved, 0x02C                                                         */
    __IO uint32_t SQR1;             /*!< ADC regular sequence register 1,                   Address offset: 0x30 */
    __IO uint32_t SQR2;             /*!< ADC regular sequence register 2,                   Address offset: 0x34 */
    __IO uint32_t SQR3;             /*!< ADC regular sequence register 3,                   Address offset: 0x38 */
    __IO uint32_t SQR4;             /*!< ADC regular sequence register 4,                   Address offset: 0x3C */
    __IO uint32_t DR;               /*!< ADC regular data register,                         Address offset: 0x40 */
    uint32_t      RESERVED3;        /*!< Reserved, 0x044                                                         */
    uint32_t      RESERVED4;        /*!< Reserved, 0x048                                                         */
    __IO uint32_t JSQR;             /*!< ADC injected sequence register,                    Address offset: 0x4C */
    uint32_t      RESERVED5[4];     /*!< Reserved, 0x050 - 0x05C                                                 */
    __IO uint32_t OFR1;             /*!< ADC offset register 1,                             Address offset: 0x60 */
    __IO uint32_t OFR2;             /*!< ADC offset register 2,                             Address offset: 0x64 */
    __IO uint32_t OFR3;             /*!< ADC offset register 3,                             Address offset: 0x68 */
    __IO uint32_t OFR4;             /*!< ADC offset register 4,                             Address offset: 0x6C */
    uint32_t      RESERVED6[4];     /*!< Reserved, 0x070 - 0x07C                                                 */
    __IO uint32_t JDR1;             /*!< ADC injected data register 1,                      Address offset: 0x80 */
    __IO uint32_t JDR2;             /*!< ADC injected data register 2,                      Address offset: 0x84 */
    __IO uint32_t JDR3;             /*!< ADC injected data register 3,                      Address offset: 0x88 */
    __IO uint32_t JDR4;             /*!< ADC injected data register 4,                      Address offset: 0x8C */
    uint32_t      RESERVED7[4];     /*!< Reserved, 0x090 - 0x09C                                                 */
    __IO uint32_t AWD2CR;           /*!< ADC  Analog Watchdog 2 Configuration Register,     Address offset: 0xA0 */
    __IO uint32_t AWD3CR;           /*!< ADC  Analog Watchdog 3 Configuration Register,     Address offset: 0xA4 */
    uint32_t      RESERVED8;        /*!< Reserved, 0x0A8                                                         */
    uint32_t      RESERVED9;        /*!< Reserved, 0x0AC                                                         */
    __IO uint32_t DIFSEL;           /*!< ADC  Differential Mode Selection Register,         Address offset: 0xB0 */
    __IO uint32_t CALFACT;          /*!< ADC  Calibration Factors,                          Address offset: 0xB4 */

    } ADC_TypeDef;

}