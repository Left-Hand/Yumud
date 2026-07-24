#pragma once

#include "core/intrinsics/mul.hpp"

namespace ymd::fxmath{

#define DEF_FXMATH_ATTRIBUTE_HOT_TABLE \
    __attribute__((section(".rodata"))) \
    alignas(4) 

#define DEF_FXMATH_ATTRIBUTE_HOT_CODE
}