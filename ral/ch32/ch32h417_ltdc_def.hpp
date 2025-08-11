#pragma once

#include <cstdint>
#include "core/io/regs.hpp"


#ifndef ASSERT_REG_IS_32BIT
#define ASSERT_REG_IS_32BIT(name) static_assert((sizeof(name) == 4),  #name " is not 4 bytes"); 
#endif

namespace ymd::ral::CH32{

struct R32_LTDC_SSCR{
    uint32_t VSH:11;
    uint32_t :5;
    uint32_t HSW:12;
    uint32_t :4;
};

struct R32_LTDC_BPCR{
    uint32_t AVBP:11;
    uint32_t :5;
    uint32_t AHBP:12;
    uint32_t :4;
};

}