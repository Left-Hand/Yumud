#pragma once

#include <cstdint>


#ifndef ASSERT_REG_IS_32BIT
#define ASSERT_REG_IS_32BIT(name) static_assert((sizeof(name) == 4),  #name " is not 4 bytes"); 
#endif

#ifndef ASSERT_REG_IS_16BIT
#define ASSERT_REG_IS_16BIT(name) static_assert((sizeof(name) == 2),  #name " is not 2 bytes"); 
#endif

namespace ymd::ral::ch32::ch32x035_iwdg{

struct [[nodiscard]] R16_IWDG_CTLR {
    uint32_t KEY:16;
    uint32_t :16;
};

struct [[nodiscard]] R16_IWDG_PSCR{
    uint32_t PR:3;
    uint32_t :29;
};

struct [[nodiscard]] R16_IWDG_RLDR{
    uint32_t RL:12;
    uint32_t :20;
};

struct [[nodiscard]] R16_IWDG_STATR{
    uint32_t PVU:1;
    uint32_t RVU:1;
    uint32_t :30;
};

struct [[nodiscard]] IWDG_Def {
    volatile R16_IWDG_CTLR CTLR;
    uint16_t :16;
    volatile R16_IWDG_PSCR PSCR;
    uint16_t :16;
    volatile R16_IWDG_RLDR RLDR;
    uint16_t :16;
    volatile R16_IWDG_STATR STATR;
    uint16_t :16;
};

}