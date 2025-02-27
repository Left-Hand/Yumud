#pragma once

#include <cstdint>

#ifndef ASSERT_REG_IS_32BIT
#define ASSERT_REG_IS_32BIT(name) static_assert((sizeof(name) == 4),  #name " is not 4 bytes"); 
#endif

#ifndef ASSERT_REG_IS_16BIT
#define ASSERT_REG_IS_16BIT(name) static_assert((sizeof(name) == 2),  #name " is not 2 bytes"); 
#endif

namespace CH32V20x{

struct R32_BKP_DATAR{
    uint32_t DATA:16;
    uint32_t :16;
};

struct R16_BKP_OCTLR{
    uint16_t CAL:7;
    uint16_t CCO:1;
    uint16_t ASOE:1;
    uint16_t ASOS:1;
    uint16_t :6;
};

struct R16_BKP_TPCTLR{
    uint16_t TPE:1;;
    uint16_t TPAL:1;;
    uint16_t :14;
};

struct R16_BKP_TPCSR{
    uint16_t CTE:1;
    uint16_t CTI:1;
    uint16_t TPIE:1;
    uint16_t :5;

    uint16_t TEF:1;
    uint16_t TIF:1;
    uint16_t :6;
};

struct BKP_Def {
    volatile R32_BKP_DATAR DATA1_10[10];
    volatile R16_BKP_OCTLR OCTLR;
    uint16_t :16;
    volatile R16_BKP_TPCTLR TPCTLR;
    uint16_t :16;
    volatile R16_BKP_TPCSR TPCSR;
    uint16_t :16;
    volatile R32_BKP_DATAR DATA11_42[32];

    constexpr volatile R32_BKP_DATAR * get_data_reg(const uint8_t index){
        if(index == 0) return nullptr;
        else if(index < 10){
            return &DATA1_10[index - 1];
        }else if(index < 42){
            return &DATA11_42[index - 11];
        }else{
            return nullptr;
        }
    }

    
};


}