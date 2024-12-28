#pragma once

#include <cstdint>

namespace hwspec::CH32V20x::AWU_Regs{

struct CSR_Reg{
    uint16_t :1;
    uint16_t AWUEN:1;
    uint16_t :14;
};

struct WR_Reg{
    uint16_t APR:6;
    uint16_t :9;
};

struct PSC_Reg{
    uint16_t TBR:4;
    uint16_t :12;
};


struct AWU_Def{
    volatile CSR_Reg CSR;
    volatile WR_Reg WR;
    volatile PSC_Reg PSC;
};

}