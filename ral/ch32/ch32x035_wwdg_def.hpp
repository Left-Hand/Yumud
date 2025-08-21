#pragma once

#include <cstdint>

namespace ymd::ral::CH32V20x::IWDG_Regs{

struct CTLR_Reg {
    uint32_t T:7;
    uint32_t WDGA:1;
    uint32_t :24;
};

struct CFGR_Reg{
    uint32_t W:7;
    uint32_t WDGTB:2;
    uint32_t EWI:1;
    uint32_t :22;
};


struct STATR_Reg{
    uint32_t EWIF:1;
};

struct IWDG_Def {
    volatile CTLR_Reg CTLR;
    volatile CFGR_Reg CFGR;
    volatile STATR_Reg STATR;
};

}