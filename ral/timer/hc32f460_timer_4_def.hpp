#pragma once

#include <cstdint>
#include <bit>

namespace ymd::ral::hc32::hc32f460{

struct TMR4_CCSR{
    // 该位指示基本计数器的计数时钟分频
    // 0000：计数时钟为PCLK1
    // 0001：计数时钟为PCLK1/2
    // 0010：计数时钟为PCLK1/4
    // 0011：计数时钟为PCLK1/8
    // 0100：计数时钟为PCLK1/16
    // 0101：计数时钟为PCLK1/32
    // 0110：计数时钟为PCLK1/64
    // 0111：计数时钟为PCLK1/128
    // 1000：计数时钟为PCLK1/256
    // 1001：计数时钟为PCLK1/512
    // 1010：计数时钟为PCLK1/1024
    uint16_t CKDIV:4;


    uint16_t CLEAR:1;
    uint16_t MODE:1;
    uint16_t STOP:1;
    uint16_t BUFEN:1;
    uint16_t IRQPEN:1;
    uint16_t IRQPF:1;
    uint16_t :3;
    uint16_t IRQZEN:1;
    uint16_t IRQZF:1;
    uint16_t ECKEN:1;
};


struct TMR4_CVPR{
    
    uint16_t ZIM:4;
    uint16_t PIM:4;
    uint16_t ZIC:4;
    uint16_t PIC:4;
};

struct TMR4_OCSRn{
    uint16_t OCEH:1;
    uint16_t OCEL:1;
    uint16_t OCPH:1;
    uint16_t OCPL:1;
    uint16_t OCIEL:1;
    uint16_t OCIFH:1;
    uint16_t OCFL:1;
    uint16_t :7;
};


struct TMR4_OCERn{
    uint16_t CHBUFEN:2;
    uint16_t CLBUFEN:2;
    uint16_t MHBUF:2;
    uint16_t MLBUF:2;
    uint16_t LMCH:1;
    
    uint16_t LMMH:1;
    uint16_t LMML:1;
    uint16_t MCECH:1;
    uint16_t MCECL:1;
    uint16_t :2;
};


struct TMR4_OCMRm{
    uint16_t OCFDCH:1;
    uint16_t OCFPKH:1;
    uint16_t OCFUCH:1;
    uint16_t OCFZRH:1;
    uint16_t OPDCH:2;
    uint16_t OPPKH:2;
    uint16_t OPUCH:2;
    uint16_t OPZRH:2;
    uint16_t OPNPKH:2;
    uint16_t OPNZRH:2;
};


struct TMR4_SCSRm{
    uint16_t BUFEN:2;
    uint16_t EVTOS:3;
    uint16_t LMC:1;
    uint16_t :2;
    uint16_t EVTMS:1;
    uint16_t EVTDS:1;
    uint16_t :2;
    uint16_t DEN:1;
    uint16_t PEN:1;
    uint16_t UEN:1;
    uint16_t ZEN:1;
};

struct TMR4_SCMRm{
    uint16_t AMC:4;
    uint16_t :2;
    uint16_t MZCE:1;
    uint16_t MPCE:1;
    uint16_t :8;
};

}