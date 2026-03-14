#pragma once

#if defined(STM32F103xB) \
    || defined(STM32F103xE) || defined(STM32F103xG) || defined(STM32F105xC) || defined(STM32F107xC) \
    || defined(CH32V20x_D8W) || defined(CH32V20x_D8) || defined(CH32V20x_D6) \
    || defined(CH32V30x_D8) || defined(CH32V30x_D8C)

#include "can/sxx32_common_can_def.hpp"
namespace ymd::ral{
    using namespace sxx32::common_can;
}

#endif
