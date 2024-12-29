#pragma once

#include <cstdint> 

namespace hwspec::CH32V00x::SLTM_Regs{

struct CLTR_Reg{
    uint32_t CEN:1;
    uint32_t UDIS:1;
    uint32_t :2;
    uint32_t DIR:1;
    uint32_t CMS:2;
    uint32_t ARPE:1;
    uint32_t SMS:3;
    uint32_t :21;
};


struct DMAINTENR_Reg{
    uint32_t OC1PE:1;
    uint32_t OC2PE:1;
    uint32_t OC3PE:1;
    uint32_t OC4PE:1;
    uint32_t :7;
    uint32_t CC3DE:1;
    uint32_t CC4DE:1;
    uint32_t :19;
};

struct SLTM_Def{
    volatile CLTR_Reg CLTR;
    volatile DMAINTENR_Reg DMAINTENR;
    volatile uint16_t CNT;
    uint16_t :16;
    volatile uint16_t ARR;
    uint16_t :16;
    volatile uint16_t CVR1;
    uint16_t :16;
    volatile uint16_t CVR2;
    uint16_t :16;
    volatile uint16_t CVR3;
    uint16_t :16;
    volatile uint16_t CVR4;
};

}