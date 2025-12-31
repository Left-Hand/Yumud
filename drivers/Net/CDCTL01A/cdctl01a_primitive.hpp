#pragma once

#include "core/io/regs.hpp"
#include "core/debug/debug.hpp"

// 数据手册
// https://dukelec.com/files/cdctl01a_datasheet_v1_cn.pdf

namespace ymd::drivers::cdctl01a{

enum class RegAddr:uint8_t{
    VERSION = 0x00,// RD 0x10硬件版本号
    CLK_CTRL = 0x01,// RD/WR 0x00时钟控制
    SETTING = 0x02,// RD/WR 0x10配置
    IDLE_WAIT_LEN = 0x04,// RD/WR 0x0a进入空闲的等待时间
    TX_PERMIT_LEN_L = 0x05,// RD/WR 0x14允许发送的等待时间(10bits)
    TX_PERMIT_LEN_H = 0x06,// RD/WR 0x00
    MAX_IDLE_LEN_L = 0x07,// RD/WR 0xc8 BS模式下的最大空闲等待时间(10bits)
    MAX_IDLE_LEN_H = 0x08,// RD/WR 0x00
    TX_PRE_LEN = 0x09,// RD/WR 0x01在TO输出之前多久使能TE(2bits)
    FILTER = 0x0b,// RD/WR 0xff本机地址
    DIV_LS_L = 0x0c,// RD/WR 0x67低速波特率设置(16bits)
    DIV_LS_H = 0x0d,// RD/WR 0x00
    DIV_HS_L = 0x0e,// RD/WR 0x67高速波特率设置(16bits)
    DIV_HS_H = 0x0f,// RD/WR 0x00
    INT_FLAG = 0x10,// RD n/a状态寄存器
    INT_MASK = 0x11,// RD/WR 0x00中断掩码寄存器
    RX = 0x14,// RD n/a读RX页
    TX = 0x15,// WR n/a写TX页
    RX_CTRL = 0x16,// WR n/a RX控制
    TX_CTRL = 0x17,// WR n/a TX控制
    RX_ADDR = 0x18,// RD/WR 0x00当前RX页读指针（极少用）
    RX_PAGE_FLAG = 0x19,// RD n/a当前RX页标记
    FILTER_M1 = 0x1a,// RD/WR 0xff组播地址过滤器1
    FILTER_M2 = 0x1b,// RD/WR 0xff组播地址过滤器2
    PLL_ML = 0x30,// RD/WR 0x12 PLLM[7:0](M:9bits)
    PLL_OD_MH = 0x31,// RD/WR 0x20 PLLOD和M[8]
    PLL_N = 0x32,// RD/WR 0x00 PLLN(5bits)
    PLL_CTRL = 0x33,// RD/WR 0x01 PLL控制
    PIN_INT_CTRL = 0x34,// RD/WR 0x00 INT脚控制
    PIN_RE_CTRL = 0x35,// RD/WR 0x00 RE脚控制
    CLK_STATUS = 0x36,// RD 0x01时钟状态寄存器
};


struct R8_HwVersion:public Reg8<>{
    static constexpr RegAddr ADDRESS = RegAddr::VERSION;
    uint8_t bits;
};

struct R8_ClockCtl:public Reg8<>{
    static constexpr RegAddr ADDRESS = RegAddr::CLK_CTRL;
    uint8_t clk_is_pll_else_osc:1;
    uint8_t :6;
    uint8_t sw_rst:1;//write 1 to reset device
};

struct R8_Setting:public Reg8<>{
    static constexpr RegAddr ADDRESS = RegAddr::SETTING;
    uint8_t enable_tote_pushpull:1;
    uint8_t invert_to:1;
    uint8_t enable_crc_handled_by_user:1;
    uint8_t enable_save_broken_data_frame:1;

    uint8_t cdbus_a_mode:1;
    uint8_t cdbus_b_mode:1;
    uint8_t full_duplex:1;
    uint8_t :1;
};


struct R8_TxPreamableLen:public Reg8<>{
    static constexpr RegAddr ADDRESS = RegAddr::TX_PRE_LEN;
    uint8_t bits;
};

struct R8_Filters{}

}