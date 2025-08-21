#pragma once

#include <cstdint>

namespace ymd::ral::CH32V20x::I2C_Regs{

struct CTLR1_Reg{
    uint16_t PE:1;
    uint16_t :3;


    uint16_t ENARP:1;
    uint16_t ENPEC:1;
    uint16_t ENGC:1;
    uint16_t NOSTRETCH:1;

    uint16_t START:1;
    uint16_t STOP:1;
    uint16_t ACK:1;
    uint16_t POS:1;
    uint16_t PEC:1;
    uint16_t :2;
    uint16_t SWRST:1;
};


struct CTLR2_Reg{
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
struct OADDR1_Reg{
    uint16_t ADD0:8;
    uint16_t ADD:2;
    uint16_t :5;
    uint16_t ADDMODE:1;
};

//I2C 地址寄存器 2
struct OADDR2_Reg{
    uint16_t ENDUAL:1;
    uint16_t ADD2:7;
    uint16_t :8;
};

//I2C 数据寄存器
struct DATAR_Reg{
    uint16_t DR:8;
    uint16_t :8;
};

//I2C 状态寄存器1
struct STAR1_Reg{
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
    uint16_t :1;
    uint16_t TIMEOUT:1;
    uint16_t SMBALERT:1;
};

//I2C 状态寄存器2
struct STAR2_Reg{
    uint16_t MSL:1;
    uint16_t BUSY:1;
    uint16_t TRA:1;
    uint16_t :1;

    uint16_t GENCALL:1;
    uint16_t :2;

    uint16_t DUALF:1;

    uint16_t PEC:8;
};

//I2C 时钟寄存器
struct CKCFRG_Reg{
    uint16_t CCR:12;
    uint16_t :2;
    uint16_t DUTY:1;
    uint16_t FS:1;
};

//I2C 上升时间寄存器
struct RTR_Reg{
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
    volatile CTLR1_Reg CTLR1;
    volatile CTLR2_Reg CTLR2;
    volatile OADDR1_Reg OADDR1;
    volatile OADDR2_Reg OADDR2;
    volatile DATAR_Reg DATAR;
    volatile STAR1_Reg STAR1;
    volatile STAR2_Reg STAR2;
    volatile CKCFRG_Reg CKCFRG;
};


static inline I2C_Def * I2C1 = (I2C_Def *)(0x40005400);

}