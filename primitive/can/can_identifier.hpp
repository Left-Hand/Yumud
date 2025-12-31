#pragma once

#if defined(CH32V20X) || defined(CH32V30X) || defined(CH32L10X)\
    || defined(STM32F10X) 
#include "platform_spec/sxx32_can_identifier.hpp"
namespace ymd::hal{
using CanIdentifier = details::SXX32_CanIdentifier;
}

#else
#error "CanIdentifier is not supported for this platform"
#endif