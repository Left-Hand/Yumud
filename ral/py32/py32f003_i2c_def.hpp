#pragma once

#include <cstdint>

#ifndef BIT_CAST
#define BIT_CAST(type, source) __builtin_bit_cast(type, (source))
#endif

namespace PF32F003{

    
struct R16_I2C_CTLR1{
    uint16_t PE:1;
    uint16_t :5;
    uint16_t ENGC:1;
    uint16_t NOSTRETCH:1;

    uint16_t START:1;
    uint16_t STOP:1;
    uint16_t ACK:1;
    uint16_t POS:1;
    uint16_t :3;
    uint16_t SWRST:1;
};


struct R16_I2C_CTLR2{
    uint16_t FREQ:5;
    uint16_t :2;
    uint16_t ITERREN:1;

    uint16_t ITEVTEN:1;
    uint16_t ITBUFEN:1;
    uint16_t DMAEN:1;
    uint16_t LAST:1;
    uint16_t :4;
};

//I2C 地址寄存器 1
struct R16_I2C_OADDR1{
    uint16_t :1;
    uint16_t ADD:7;
    uint16_t :6;
};

//I2C 数据寄存器
struct R16_I2C_DATAR{
    uint16_t DR:8;
    uint16_t :8;
};

//I2C 状态寄存器1
struct R16_I2C_STAR1{
    uint16_t SB:1;
    uint16_t ADDR:1;
    uint16_t BTF:1;
    uint16_t ADD10:1;
    uint16_t STOPF:1;
    uint16_t :1;
    uint16_t RXNE:1;
    uint16_t TXE:1;
    uint16_t BERR:1;
    uint16_t ARL0:1;
    uint16_t AF:1;
    uint16_t OVR:1;
    uint16_t PECERR:1;
    uint16_t :3;
};

//I2C 状态寄存器2
struct R16_I2C_STAR2{
    uint16_t MSL:1;
    uint16_t BUSY:1;
    uint16_t TRA:1;
    uint16_t :1;

    uint16_t GENCALL:1;
    uint16_t PEC:11;
};


//I2C 时钟寄存器
struct R16_I2C_CKCFRG{
    uint16_t CCR:12;
    uint16_t :2;
    uint16_t DUTY:1;
    uint16_t FS:1;
};

//I2C 上升时间寄存器
struct R16_I2C_RTR{
    // 最大上升时间域。这个位设置主模式的 SCL 的
    // 上升时间。最大的上升沿时间等于 TRISE-1 个
    // 时钟周期。此位只能在 PE 清零下设置。比如如
    // 果 I2C 模块的输入时钟周期为 125nS，而 TRISE
    // 的值为 9，那么最大上升沿时间为（9-1）
    // *125nS，即 1000nS。
    uint16_t TRISE:6;
    uint16_t :10;
};

struct I2C_Def{
    volatile R16_I2C_CTLR1 CTLR1;
    uint16_t :16;
    volatile R16_I2C_CTLR2 CTLR2;
    uint16_t :16;
    volatile R16_I2C_OADDR1 OADDR1;
    uint16_t :16;
    uint16_t :16;
    uint16_t :16;
    volatile R16_I2C_DATAR DATAR;
    uint16_t :16;
    volatile R16_I2C_STAR1 STAR1;
    uint16_t :16;
    volatile R16_I2C_STAR2 STAR2;
    uint16_t :16;
    volatile R16_I2C_CKCFRG CKCFRG;
    uint16_t :16;
    volatile R16_I2C_RTR RTR;
    uint16_t :16;


    constexpr void enable(const Enable en){
        this->CTLR1.PE = en;
    }

    constexpr void enable_dma(const Enable en){
        this->CTLR2.DMAEN = en;
    }

    constexpr void set_next_dma_is_last(const Enable en){
        CTLR2.LAST = en;
    }

    constexpr void generate_start(const Enable en){
        CTLR1.START = en;
    }

    constexpr void generate_stop(const Enable en){
        CTLR1.STOP = en;
    }

    constexpr void enable_ack(const Enable en){
        CTLR1.ACK = en;
    }

    constexpr void generate_call(const Enable en){
        CTLR1.ENGC = en;
    }

    constexpr void enable_interrupt(
        const bool err_en,
        const bool event_en,
        const bool buf_en
    ){
        CTLR2.ITERREN = err_en;
        CTLR2.ITEVTEN = event_en;
        CTLR2.ITBUFEN = buf_en;
    }

    constexpr void send(const uint8_t data){
        DATAR.DR = data;
    }

    constexpr uint8_t receive(){
        return uint8_t(DATAR.DR);
    }

    constexpr void master_send_7bit_addr(const uint8_t addr){
        DATAR.DR = addr << 1;
    }

    constexpr void slave_send_7bit_addr(const uint8_t addr){
        DATAR.DR = (addr << 1) | 0x01;
    }

    constexpr void send_8bit_addr(const uint8_t addr){
        DATAR.DR = addr;
    }

    constexpr void soft_reset(const Enable en){
        CTLR1.SWRST = en;
    }

    constexpr void nack_next_transmit(){
        CTLR1.POS = 1;
    }

    constexpr void nack_curr_transmit(){
        CTLR1.POS = 0;
    }

    constexpr uint8_t get_pec(){
        return STAR2.PEC;
    }

    constexpr void set_stretch(const bool set){
        CTLR1.NOSTRETCH = !set;
    }

    constexpr void set_dutycycle_16_9(const bool set){
        CKCFRG.DUTY = set;
    }

    struct Events{
        uint32_t SB:1;
        uint32_t ADDR:1;
        uint32_t BTF:1;
        uint32_t ADD10:1;
        uint32_t STOPF:1;
        uint32_t :1;
        uint32_t RXNE:1;
        uint32_t TXE:1;
        uint32_t BERR:1;
        uint32_t ARL0:1;
        uint32_t AF:1;
        uint32_t OVR:1;
        uint32_t PECERR:1;
        uint32_t :1;
        uint32_t TIMEOUT:1;
        uint32_t SMBALERT:1;
        uint32_t MSL:1;
        uint32_t BUSY:1;
        uint32_t TRA:1;
        uint32_t :1;
    
        uint32_t GENCALL:1;
        uint32_t SMBDEFAULT:1;
        uint32_t SMBHOST:1;
        uint32_t DYAKF:1;

        bool check(const Events eve){
            return( 
                BIT_CAST(uint32_t, *this)
                & BIT_CAST(uint32_t, eve)
            );
        }

        uint16_t low16() const{
            return uint16_t(BIT_CAST(uint32_t, *this));
        }

        uint8_t high8() const{
            return uint8_t(BIT_CAST(uint32_t, *this) >> 16);
        }
    };

    constexpr auto get_events(){
        return BIT_CAST(Events,
            uint32_t(BIT_CAST(uint16_t, STAR1))
            | (BIT_CAST(uint16_t, STAR2) << 16)
        );
    }

    constexpr void clear_events(const Events events){
        const auto low16 = events.low16();
        const auto high8 = events.high8();

        if(low16){
            const_cast<R16_I2C_STAR1 &>(STAR1) = 
                BIT_CAST(R16_I2C_STAR1, uint16_t(BIT_CAST(uint16_t, STAR1) & (~low16)));
        }

        if(high8){
            const_cast<R16_I2C_STAR2 &>(STAR2) = 
            BIT_CAST(R16_I2C_STAR2, uint16_t(BIT_CAST(uint16_t, STAR2) & ((~high8) | 0xff00)));
        }
    }
};


static inline I2C_Def * I2C1_Inst = (I2C_Def *)(0x40005400);
static inline I2C_Def * I2C2_Inst = (I2C_Def *)(0x40005800);

}