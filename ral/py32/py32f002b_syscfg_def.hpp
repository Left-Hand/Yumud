#pragma once


#include <cstdint>
#include "core/io/regs.hpp"


namespace ymd::ral::py32::py32f002b{


struct SYSCFG_CFGR1{
    uint32_t MEM_MODE:2;
    uint32_t :14;
    uint32_t I2C_PA2_ANF:1;
    uint32_t I2C_PB3_ANF:1;
    uint32_t I2C_PB4_ANF:1;
    uint32_t I2C_PB6_ANF:1;
    uint32_t :12;
};



}