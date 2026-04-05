#pragma once

#include "core/intrinsics/mul.hpp"
#include "core/intrinsics/mask.hpp"

namespace ymd::fxmath{

#define FXMATH_ATTRIBUTE_HOT_TABLE \
    __attribute__((section(".rodata"))) \
    alignas(4) 

}