#pragma once

#if \
    defined(APM32F465xE) \
    || defined(APM32F423)  || defined(APM32F425) || defined(APM32F427)\
    \
    || defined(AT32F435)  || defined(AT32F437)\
    \
    || defined(CH32V20x_D8W) || defined(CH32V20x_D8) || defined(CH32V20x_D6) \
    || defined(CH32V30x_D8) || defined(CH32V30x_D8C) \
    \
    || defined(GD32F205xx) || defined(GD32F207xx)\
    || defined(GD32F405xx) || defined(GD32F407xx) || defined(GD32F425xx)\
    || defined(GD32F427xx) || defined(GD32F450xx) || defined(GD32F470xx)\
    \
    || defined(MH245X) \
    \
    || defined(N32G452) || defined(N32G455)\
    || defined(N32L401) || defined(N32L43x)\
    \
    || defined(STM32F0xx) \
    || defined(STM32F1xx) || defined(STM32F103xG) || defined(STM32F105xC) || defined(STM32F107xC) \
    || defined(STM32F30x) || defined(STM32F334xx) || defined(STM32F37x) \
    \
    || defined(STM32F4xx) \
    || defined(STM32F401xx) || defined(STM32F405xx) || defined(STM32F407xx) \
    || defined(STM32F410xx) || defined(STM32F411xx) || defined(STM32F412xx) \
    || defined(STM32F415xx) || defined(STM32F417xx) || defined(STM32F427xx) \
    || defined(STM32F429xx) || defined(STM32F437xx) || defined(STM32F439xx) \
    || defined(STM32F446xx) || defined(STM32F469xx)\
    \
    || defined(STM32F7xx) \
    || defined(STM32F745xx) || defined(STM32F746xx) || defined(STM32F756xx) \
    || defined(STM32F765xx) || defined(STM32F767xx) || defined(STM32F769xx) \
    || defined(STM32F777xx) || defined(STM32F779xx)\
    \
    || defined(STM32L4xx)\

#ifndef CH32L103
#include "can/sxx32_common_can_def.hpp"
namespace ymd::ral{
using namespace can::sxx32_common;

[[maybe_unused]] static inline CAN_Def * CAN1_Inst = reinterpret_cast<CAN_Def *>(0x40006400);
[[maybe_unused]] static inline CAN_Def * CAN2_Inst = reinterpret_cast<CAN_Def *>(0x40006800);
[[maybe_unused]] static inline CAN_Filter_Def * CAN_Filt = reinterpret_cast<CAN_Filter_Def *>(0x40006600);
}

static constexpr uint8_t CAN_MAX_SWJ_BITS = 0x03;
static constexpr uint8_t CAN_MAX_BS1_BITS = 0x0f;
static constexpr uint8_t CAN_MAX_BS2_BITS = 0x07;
#else

#define YMD_CANFD_SPECIFIC_CH32L103
#include "can/ch32l103_canfd_def.hpp"
namespace ymd::ral{
using namespace can::ch32l103_specified;

[[maybe_unused]] static inline CAN_Def * CAN1_Inst = reinterpret_cast<CAN_Def *>(0x40006400);
[[maybe_unused]] static inline CAN_Filter_Def * CAN_Filt = reinterpret_cast<CAN_Filter_Def *>(0x40006600);

static constexpr uint8_t CAN_MAX_SWJ_BITS = 0x03;
static constexpr uint8_t CAN_MAX_BS1_BITS = 0x0f;
static constexpr uint8_t CAN_MAX_BS2_BITS = 0x0f;
}
#endif
#endif

#if \
    defined(STM32G4xx)\
    || defined(STM32H5xx)\
    || defined(STM32U3xx)\

//fdcan v1
//TODO
#endif

#if \
    defined(STM32H7xx)\

//fdcan v2
//TODO
#endif

#if \
    defined (GD32E50x)

//TODO
#endif



#if \
    defined (GD32A50x) 

//TODO
#endif