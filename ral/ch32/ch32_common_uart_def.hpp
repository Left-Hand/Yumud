#pragma once

#include <cstdint>
#include "core/io/regs.hpp"


#ifndef BIT_CAST
#define BIT_CAST(type, source) __builtin_bit_cast(type, (source))
#endif

namespace ymd::ral::CH32{

struct R16_STATR{
    uint16_t PE:1;
    uint16_t FE:1;
    uint16_t NE:1;
    uint16_t ORE:1;

    uint16_t IDLE:1;
    uint16_t RXNE:1;
    uint16_t TC:1;
    uint16_t TXE:1;

    uint16_t LBD:1;
    uint16_t CTS:1;

    uint16_t __RESV__:6;
};   

struct R16_DATAR{
    uint16_t DR:9;
    uint16_t :7;
};

struct R16_BRR{
    uint16_t FRAC:4;
    uint16_t MANT:12;
};

struct R16_CTLR1{
    uint16_t SBK:1;
    uint16_t RWU:1;
    uint16_t RE:1;
    uint16_t TE:1;

    uint16_t IDLEIE:1;
    uint16_t RXNEIE:1;
    uint16_t TCIE:1;
    uint16_t TXEIE:1;

    uint16_t PEIE:1;
    uint16_t PS:1;
    uint16_t PCE:1;
    uint16_t WAKE:1;

    uint16_t M:1;
    uint16_t UE:1;
    uint16_t :2;
};

struct R16_CTLR2{
    uint16_t ADD:4;
    uint16_t :1;
    uint16_t LBDL:1;
    uint16_t LBDIE:1;
    uint16_t :1;

    uint16_t LBCL:1;
    uint16_t CPHA:1;
    uint16_t CPOL:1;
    uint16_t CLKEN:1;
    uint16_t STOP:2;
    uint16_t LINEN:1;
};

struct R16_CTLR3{
    uint16_t EIE:1;
    uint16_t IREN:1;
    uint16_t IRLP:1;
    uint16_t HDSEL:1;

    uint16_t NACK:1;
    uint16_t SCEN:1;
    uint16_t DMAR:1;
    uint16_t DMAT:1;

    uint16_t RTSE:1;
    uint16_t CTSE:1;
    uint16_t CTSIE:1;
    uint16_t :5;
};

struct R16_GPR{
    uint16_t PSC:8;
    uint16_t GT:8;
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
    volatile R16_GPR GPR;
    uint16_t :16;

    void enable(const Enable en){
        CTLR1.UE = en == EN;
    }

    void enable_tx_dma(const Enable en){
        CTLR3.DMAT = en == EN;
    }

    void enable_rx_dma(const Enable en){
        CTLR3.DMAR = en == EN;
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
        CTLR1.RWU = en == EN;
    }

    void switch_lin_breakdetect_11bit(){
        CTLR2.LBCL = 1;
    }

    void switch_lin_breakdetect_10bit(){
        CTLR2.LBCL = 0;
    }

    void enable_lin(const Enable en){
        CTLR2.LINEN = en == EN;
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

    void set_guard_time(const uint8_t time){
        GPR.GT = time;
    }

    void set_prescaler(const uint8_t scaler){
        GPR.PSC = scaler;
    }

    void enable_smartcard(const Enable en){
        CTLR3.SCEN = en == EN;
    }

    void enable_smartcard_nack(const Enable en){
        CTLR3.NACK = en == EN;
    }

    void enable_halfduplex(const Enable en){
        CTLR3.HDSEL = en == EN;
    }

    void enable_oversamp8(const Enable en){
        // CTLR1.
    }

    void enable_irda_lowpower(const Enable en){
        CTLR3.IRLP = en == EN;
    }

    void enable_irda(const Enable en){
        CTLR3.IREN = en == EN;
    }

    void wait_transmit_complete(){
        while(!STATR.TC);
    }
    
    void wait_transmit_start(){
        while(!STATR.TXE);
    }

    
    R16_STATR get_events(){
        return std::bit_cast<R16_STATR>(STATR);
    }

    constexpr void clear_events(const R16_STATR events){
        // STATR =  BIT_CAST(R16_STATR, uint16_t(BIT_CAST(uint16_t, STATR) & (~BIT_CAST(uint16_t, events))));
        const_cast<R16_STATR &>(this->STATR) = std::bit_cast<R16_STATR>(
            uint16_t(std::bit_cast<uint16_t>(STATR) & (~std::bit_cast<uint16_t> (events)))
        );
    }
};

[[maybe_unused]] static inline USART_Def * USART1_Inst = (USART_Def *)(0x40013800);
[[maybe_unused]] static inline USART_Def * USART2_Inst = (USART_Def *)(0x40004400);
[[maybe_unused]] static inline USART_Def * USART3_Inst = (USART_Def *)(0x40004800);
[[maybe_unused]] static inline USART_Def * USART4_Inst = (USART_Def *)(0x40004C00);
[[maybe_unused]] static inline USART_Def * USART5_Inst = (USART_Def *)(0x40005000);
[[maybe_unused]] static inline USART_Def * USART6_Inst = (USART_Def *)(0x40001800);
[[maybe_unused]] static inline USART_Def * USART7_Inst = (USART_Def *)(0x40001C00);
[[maybe_unused]] static inline USART_Def * USART8_Inst = (USART_Def *)(0x40002000);
}