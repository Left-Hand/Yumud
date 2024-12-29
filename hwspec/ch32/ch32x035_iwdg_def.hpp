#pragma once

#include <cstdint>

namespace hwspec::CH32V20x::IWDG_Regs{

struct CTLR_Reg {
    uint32_t KEY:16;
    uint32_t :16;
};

struct PSCR_Reg{
    uint32_t PR:3;
    uint32_t :29;
};

struct RLDR_Reg{
    uint32_t RL:12;
    uint32_t :20;
};

struct STATR_Reg{
    uint32_t PVU:1;
    uint32_t RVU:1;
    uint32_t :30;
};

struct IWDG_Def {
    volatile CTLR_Reg CTLR;
    volatile PSCR_Reg PSCR;
    volatile RLDR_Reg RLDR;
    volatile STATR_Reg STATR;
};

}