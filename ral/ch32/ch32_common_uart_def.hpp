#pragma once

#include <cstdint>
#include "core/io/regs.hpp"


#ifndef BIT_CAST
#define BIT_CAST(type, source) __builtin_bit_cast(type, (source))
#endif

namespace ymd::ral::ch32::common_uart{

struct R32_STATR{
    // 校验错误标志。在接收模式下，如果产生奇偶
    // 检验错误，硬件置位此位。读此位再读数据寄
    // 存器的操作会复位此位。在清除此位前，软件
    // 必须等 RXNE 标志位被置位。如果 PEIE 之前已
    // 经被置位，那么此位被置位会产生对应的中
    // 断。
    uint32_t PE:1;

    // 帧错误标志。当检测到同步错误，过多的噪声
    // 或者断开符，该位将会被硬件置位。读此位再
    // 读数据寄存器的操作会复位此位。
    uint32_t FE:1;

    // 噪声错误标志。当检测到噪声错误标志时，由
    // 硬件置位。读状态寄存器后，再读数据寄存器
    // 的操作会复位此位。
    uint32_t NE:1;

    // 过载错误标志。当接收移位寄存器存在数据需
    // 要转到数据寄存器时，但是数据寄存器的接收
    // 域还有数据未读出时，此位将会被置位。如果
    // RXNEIE 被置位了，还会产生对应中断。
    uint32_t ORE:1;

    uint32_t IDLE:1;
    uint32_t RXNE:1;
    uint32_t TC:1;
    uint32_t TXE:1;

    // LIN Break 检测标志。当检测到 LIN Break 时，
    // 该位被硬件置位。由软件清零。如果 LBDIE 已
    // 经被置位，则将会产生中断。
    uint32_t LBD:1;

    // CTS 状态改变标志。如果设置了 CTSE 位，当
    // nCTS 输出状态改变时，该位将由硬件置高。由
    // 软件清零。如果 CTSIE 位已经被置位，则会产
    // 生中断。
    uint32_t CTS:1;

    uint32_t __RESV__:22;

    constexpr bool any_fault() const {
        return bool(std::bit_cast<uint32_t>(*this) & 0x0f);
    }
};   

struct R32_DATAR{
    uint32_t DR:9;
    uint32_t :23;
};

struct R32_BRR{
    uint32_t FRAC:4;
    uint32_t MANT:28;
};

struct R32_CTLR1{
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

struct R32_CTLR2{
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

struct R32_CTLR3{
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
    uint32_t :21;
};

struct R32_GPR{
    uint32_t PSC:8;
    uint32_t GT:8;
    uint32_t :16;
};

struct USART_Def{
    volatile R32_STATR STATR;
    volatile R32_DATAR DATAR;
    volatile R32_BRR BRR;
    volatile R32_CTLR1 CTLR1;
    volatile R32_CTLR2 CTLR2;
    volatile R32_CTLR3 CTLR3;
    volatile R32_GPR GPR;

    static_assert(sizeof(R32_STATR) == 4);
    static_assert(sizeof(R32_DATAR) == 4);
    static_assert(sizeof(R32_BRR) == 4);
    static_assert(sizeof(R32_CTLR1) == 4);
    static_assert(sizeof(R32_CTLR2) == 4);
    static_assert(sizeof(R32_CTLR3) == 4);
    static_assert(sizeof(R32_GPR) == 4);

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

    
    R32_STATR get_events(){
        return std::bit_cast<R32_STATR>(STATR);
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