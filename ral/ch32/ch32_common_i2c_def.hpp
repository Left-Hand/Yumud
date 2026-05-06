#pragma once

#include <cstdint>
#include "core/io/regs.hpp"

#ifndef BIT_CAST
#define BIT_CAST(type, source) __builtin_bit_cast(type, (source))
#endif

namespace ymd::ral::ch32::common_i2c{


struct [[nodiscard]] R16_I2C_CTLR1{
    uint16_t PE:1;
    uint16_t SMBUS:1;
    uint16_t __resv__:1;
    uint16_t SMBTYPE:1;


    uint16_t ENARP:1;
    uint16_t ENPEC:1;
    uint16_t ENGC:1;
    uint16_t NOSTRETCH:1;

    uint16_t START:1;
    uint16_t STOP:1;
    uint16_t ACK:1;
    uint16_t POS:1;
    uint16_t PEC:1;
    uint16_t ALERT:1;
    uint16_t __resv2__:1;
    uint16_t SWRST:1;
};


struct [[nodiscard]] R16_I2C_CTLR2{
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
struct [[nodiscard]] R16_I2C_OADDR1{
    uint16_t ADD0:1;
    uint16_t ADD:9;
    uint16_t :5;
    uint16_t ADDMODE:1;
};

//I2C 地址寄存器 2
struct [[nodiscard]] R16_I2C_OADDR2{
    uint16_t ENDUAL:1;
    uint16_t ADD2:7;
    uint16_t :8;
};

//I2C 数据寄存器
struct [[nodiscard]] R16_I2C_DATAR{
    uint16_t DR;
};

//I2C 状态寄存器1
struct [[nodiscard]] R16_I2C_STAR1{
    uint16_t SB:1;
    uint16_t ADDR:1;
    uint16_t BTF:1;
    uint16_t ADD10:1;
    uint16_t STOPF:1;
    uint16_t __resv__:1;
    uint16_t RXNE:1;
    uint16_t TXE:1;
    uint16_t BERR:1;
    uint16_t ARL0:1;
    uint16_t AF:1;
    uint16_t OVR:1;
    uint16_t PECERR:1;
    uint16_t __resv2__:1;
    uint16_t TIMEOUT:1;
    uint16_t SMBALERT:1;
};

//I2C 状态寄存器2
struct [[nodiscard]] R16_I2C_STAR2{
    uint8_t MSL:1;
    uint8_t BUSY:1;
    uint8_t TRA:1;
    uint8_t __resv__:1;

    uint8_t GENCALL:1;
    uint8_t SMBDEFAULT:1;
    uint8_t SMBHOST:1;
    uint8_t DYAKF:1;

    uint8_t PEC;

};


//I2C 时钟寄存器
struct [[nodiscard]] R16_I2C_CKCFRG{
    uint16_t CCR:12;
    uint16_t :2;
    uint16_t DUTY:1;
    uint16_t FS:1;
};

//I2C 上升时间寄存器
struct [[nodiscard]] R16_I2C_RTR{
    // 最大上升时间域。这个位设置主模式的 SCL 的
    // 上升时间。最大的上升沿时间等于 TRISE-1 个
    // 时钟周期。此位只能在 PE 清零下设置。比如如
    // 果 I2C 模块的输入时钟周期为 125nS，而 TRISE
    // 的值为 9，那么最大上升沿时间为（9-1）
    // *125nS，即 1000nS。
    uint16_t TRISE:6;
    uint16_t :10;
};

struct [[nodiscard]] I2C_Def{
    volatile R16_I2C_CTLR1 CTLR1;
    uint16_t :16;
    volatile R16_I2C_CTLR2 CTLR2;
    uint16_t :16;
    volatile R16_I2C_OADDR1 OADDR1;
    uint16_t :16;
    volatile R16_I2C_OADDR2 OADDR2;
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


    void enable(const Enable en){
        this->CTLR1.PE = (en == EN);
    }

    void enable_dma(const Enable en){
        this->CTLR2.DMAEN = (en == EN);
    }

    void set_next_dma_is_last(const Enable en){
        CTLR2.LAST = (en == EN);
    }

    void generate_start(const Enable en){
        CTLR1.START = (en == EN);
    }

    void generate_stop(const Enable en){
        CTLR1.STOP = (en == EN);
    }

    void enable_ack(const Enable en){
        CTLR1.ACK = (en == EN);
    }

    void set_address2(const uint8_t addr){
        OADDR2.ADD2 = addr;
    }

    void enable_dual_address(const Enable en){
        OADDR2.ENDUAL = (en == EN);
    }

    void generate_call(const Enable en){
        CTLR1.ENGC = (en == EN);
    }


    void enable_interrupt(
        const Enable err_en,
        const Enable event_en,
        const Enable buf_en
    ){
        CTLR2.ITERREN = (err_en == EN);
        CTLR2.ITEVTEN = (event_en == EN);
        CTLR2.ITBUFEN = (buf_en == EN);
    }

    void send(const uint8_t data){
        DATAR.DR = data;
    }

    [[nodiscard]] uint16_t receive(){
        return static_cast<uint16_t>(DATAR.DR);
    }

    void master_send_7bit_addr(const uint8_t addr){
        DATAR.DR = addr << 1;
    }

    void slave_send_7bit_addr(const uint8_t addr){
        DATAR.DR = (addr << 1) | 0x01;
    }

    void send_8bit_addr(const uint8_t addr){
        DATAR.DR = addr;
    }

    void soft_reset(const Enable en){
        CTLR1.SWRST = (en == EN);
    }

    void nack_next_transmit(){
        CTLR1.POS = 1;
    }

    void nack_curr_transmit(){
        CTLR1.POS = 0;
    }

    void smbus_alert_level(const bool level){
        CTLR1.ALERT = !level;
    }

    void transmit_pec(const Enable en){
        CTLR1.PEC = (en == EN);
    }

    void calculate_pec(const Enable en){
        CTLR1.ENPEC = (en == EN);
    }

    uint8_t get_pec(){
        return std::bit_cast<uint8_t>(STAR2.PEC);
    }

    void enable_arp(const Enable en){
        CTLR1.ENARP = (en == EN);
    }

    void set_stretch(const bool set){
        CTLR1.NOSTRETCH = !set;
    }

    void set_dutycycle_16_9(const bool set){
        CKCFRG.DUTY = set;
    }

    struct [[nodiscard]] Events{
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

        constexpr bool check(const Events eve){
            return( 
                BIT_CAST(uint32_t, *this)
                & BIT_CAST(uint32_t, eve)
            );
        }

        constexpr uint16_t low16() const noexcept {
            return uint16_t(std::bit_cast<uint32_t> (*this));
        }

        constexpr uint8_t high8() const noexcept {
            return uint8_t(std::bit_cast<uint32_t> (*this) >> 16);
        }
    };

    constexpr auto get_events(){
        return BIT_CAST(Events,
            uint32_t(std::bit_cast<uint16_t>(STAR1))
            | (std::bit_cast<uint16_t>(STAR2) << 16)
        );
    }
};


[[maybe_unused]] static inline I2C_Def * I2C1_Inst = reinterpret_cast<I2C_Def *>(0x40005400);
[[maybe_unused]] static inline I2C_Def * I2C2_Inst = reinterpret_cast<I2C_Def *>(0x40005800);

}