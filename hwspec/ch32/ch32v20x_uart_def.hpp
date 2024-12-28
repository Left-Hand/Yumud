#pragma once

#include <cstdint>

namespace hwspec::CH32V20x::UART_Regs{

struct STATR_Reg{
    uint32_t PE:1;
    uint32_t FE:1;
    uint32_t NE:1;
    uint32_t ORE:1;
    uint32_t IDLE:1;
    uint32_t RXNE:1;
    uint32_t TC:1;
    uint32_t TXE:1;
    uint32_t LBD:1;
    uint32_t CTS:1;
    uint32_t :22;
};   

struct DATAR_Reg{
    uint32_t DR:9;
    uint32_t :23;
};

struct BRR_Reg{
    uint32_t FRAC:4;
    uint32_t MANT:12;
};

struct CTLR1_Reg{
    uint32_t SBK:1;
    uint32_t RWU:1;
    uint32_t RE:1;
    uint32_t TE:1;

    uint32_t IDLEIE:1;
    uint32_t RXNEIE:1;
    uint32_t TCIE:1;
    uint32_t TXEIE:1;

    uint32_t PEIE:1;
    uint32_t PS:1;
    uint32_t PCE:1;
    uint32_t WAKE:1;

    uint32_t M:1;
    uint32_t UE:1;
    uint32_t :18;
};


struct CTLR2_Reg{
    uint32_t ADD:4;
    uint32_t :1;
    uint32_t LBDL:1;
    uint32_t LBDIE:1;
    uint32_t :1;

    uint32_t LBCL:1;
    uint32_t CPHA:1;
    uint32_t CPOL:1;
    uint32_t CLKEN:1;
    uint32_t STOP:2;
    uint32_t LINEN:1;
    uint32_t :16;
};

struct CTLR3_Reg{
    uint32_t EIE:1;
    uint32_t IREN:1;
    uint32_t IRLP:1;
    uint32_t HDSEL:1;
    uint32_t NACK:1;
    uint32_t SCEN:1;
    uint32_t DMAR:1;
    uint32_t DMAT:1;
    uint32_t RTSE:1;
    uint32_t CTSE:1;
    uint32_t CTSIE:1;
    uint32_t :31;
};

struct GPR_Reg{
    uint32_t PSC:8;
    uint32_t GT:8;
    uint32_t :16;
};

struct UART_Def{
    volatile STATR_Reg STATR;
    volatile DATAR_Reg DATAR;
    volatile BRR_Reg BRR;
    volatile CTLR1_Reg CTLR1;
    volatile CTLR2_Reg CTLR2;
    volatile CTLR3_Reg CTLR3;
    volatile GPR_Reg GPR;
};

static inline UART_Def * UART1 = (UART_Def *)(0x40013800);
static inline UART_Def * UART2 = (UART_Def *)(0x40004400);
static inline UART_Def * UART3 = (UART_Def *)(0x40014800);
static inline UART_Def * UART4 = (UART_Def *)(0x40004C00);
static inline UART_Def * UART5 = (UART_Def *)(0x40015000);
static inline UART_Def * UART6 = (UART_Def *)(0x40001800);
static inline UART_Def * UART7 = (UART_Def *)(0x40011C00);
static inline UART_Def * UART8 = (UART_Def *)(0x40002000);

}