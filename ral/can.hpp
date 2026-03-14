#pragma once

#if defined(STM32F103xB) \
    || defined(STM32F103xE) || defined(STM32F103xG) || defined(STM32F105xC) || defined(STM32F107xC) \
    || defined(CH32V20x_D8W) || defined(CH32V20x_D8) || defined(CH32V20x_D6) \
    || defined(CH32V30x_D8) || defined(CH32V30x_D8C)

// #include "can/sxx32_common_can_def.hpp"
// namespace ymd::ral{
// using namespace can::sxx32_common;

// [[maybe_unused]] static inline CAN_Def * CAN1_Inst = reinterpret_cast<CAN_Def *>(0x40006400);
// [[maybe_unused]] static inline CAN_Def * CAN2_Inst = reinterpret_cast<CAN_Def *>(0x40006800);
// [[maybe_unused]] static inline CAN_Filter_Def * CAN_Filt = reinterpret_cast<CAN_Filter_Def *>(0x40006600);
// }

// #elif defined(CH32L103)
#include "can/ch32l103_can_def.hpp"
namespace ymd::ral{
using namespace can::ch32l103_specified;

[[maybe_unused]] static inline CAN_Def * CAN1_Inst = reinterpret_cast<CAN_Def *>(0x40006400);
[[maybe_unused]] static inline CAN_Filter_Def * CAN_Filt = reinterpret_cast<CAN_Filter_Def *>(0x40006600);

}
#endif
