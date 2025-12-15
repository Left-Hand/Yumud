#pragma once

#include <cstdint>

namespace ymd::ral::py32::py32f003{

struct R16_STATR{
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
    uint32_t :6;
};   

struct R16_DATAR{
    uint32_t DR:9;
    uint32_t :7;
};

struct R16_BRR{
    uint32_t FRAC:4;
    uint32_t MANT:12;
};

struct R16_CTLR1{
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
    uint32_t :2;
};


struct R16_CTLR2{
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
};

struct R16_CTLR3{
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
    uint32_t :5;
};


struct USART_Def{
    volatile R16_STATR STATR;
    uint16_t :16;
    volatile R16_DATAR DATAR;
    uint16_t :16;
    volatile R16_BRR BRR;
    uint16_t :16;
    volatile R16_CTLR1 CTLR1;
    uint16_t :16;
    volatile R16_CTLR2 CTLR2;
    uint16_t :16;
    volatile R16_CTLR3 CTLR3;
    uint16_t :16;
    void enable(const Enable en){
        CTLR1.UE = en;
    }

    void enable_tx_dma(const Enable en){
        CTLR3.DMAT = en;
    }

    void enable_rx_dma(const Enable en){
        CTLR3.DMAR = en;
    }

    void set_address(const uint8_t addr){
        CTLR2.ADD = addr;
    }
    
    void switch_wakeup_to_addressmark(){
        CTLR1.WAKE = 1;
    }

    void switch_wakeup_to_idleline(){
        CTLR1.WAKE = 0;
    }

    void enable_slave_wakeup(const Enable en){
        CTLR1.RWU = en;
    }

    void switch_lin_breakdetect_11bit(){
        CTLR2.LBCL = 1;
    }

    void switch_lin_breakdetect_10bit(){
        CTLR2.LBCL = 0;
    }

    void enable_lin(const Enable en){
        CTLR2.LINEN = en;
    }

    void send(const uint16_t data){
        DATAR.DR = data & 0x1ff;
    }

    uint16_t receive(){
        return (DATAR.DR) & 0x1ff;
    }

    void send_break(){
        CTLR1.SBK = 1;
    }


    void enable_smartcard(const Enable en){
        CTLR3.SCEN = en;
    }

    void enable_smartcard_nack(const Enable en){
        CTLR3.NACK = en;
    }

    void enable_halfduplex(const Enable en){
        CTLR3.HDSEL = en;
    }

    void enable_oversamp8(const Enable en){
        // CTLR1.
    }

    void switch_irda_lowpower(){
        CTLR3.IRLP = 1;
    }

    void switch_irda_lowpower(){
        CTLR3.IRLP = 0;
    }

    void enable_irda(const Enable en){
        CTLR3.IREN = en;
    }

    
    
};

static inline USART_Def * UART1_Inst = reinterpret_cast<USART_Def *>(0x40013800);
static inline USART_Def * UART2_Inst = reinterpret_cast<USART_Def *>(0x40004400);
static inline USART_Def * UART3_Inst = reinterpret_cast<USART_Def *>(0x40014800);
static inline USART_Def * UART4_Inst = reinterpret_cast<USART_Def *>(0x40004C00);
static inline USART_Def * UART5_Inst = reinterpret_cast<USART_Def *>(0x40015000);
static inline USART_Def * UART6_Inst = reinterpret_cast<USART_Def *>(0x40001800);
static inline USART_Def * UART7_Inst = reinterpret_cast<USART_Def *>(0x40011C00);
}