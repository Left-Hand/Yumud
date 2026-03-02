#pragma once

#include "core/io/regs.hpp"
#include "core/debug/debug.hpp"

// 数据手册
// https://dukelec.com/files/cdctl01a_datasheet_v1_cn.pdf

namespace ymd::drivers::cdctl01a{


struct CDCTL01A_Prelude{

enum class RegAddr:uint8_t{
    HwVersion = 0x00,              // RD 0x10 硬件版本号
    ClkCtrl = 0x01,              // RD/WR 0x00 时钟控制
    Setting = 0x02,              // RD/WR 0x10 配置
    IdleWaitLen = 0x04,          // RD/WR 0x0a 进入空闲的等待时间
    TxPermitLenL = 0x05,         // RD/WR 0x14 允许发送的等待时间 (10bits)
    TxPermitLenH = 0x06,         // RD/WR 0x00
    MaxIdleLenL = 0x07,          // RD/WR 0xc8 BS 模式下的最大空闲等待时间 (10bits)
    MaxIdleLenH = 0x08,          // RD/WR 0x00
    TxPreLen = 0x09,             // RD/WR 0x01 在 TO 输出之前多久使能 TE(2bits)
    Filter = 0x0b,               // RD/WR 0xff 本机地址
    DivLsL = 0x0c,               // RD/WR 0x67 低速波特率设置 (16bits)
    DivLsH = 0x0d,               // RD/WR 0x00
    DivHsL = 0x0e,               // RD/WR 0x67 高速波特率设置 (16bits)
    DivHsH = 0x0f,               // RD/WR 0x00
    IntFlag = 0x10,              // RD n/a状态寄存器
    IntMask = 0x11,              // RD/WR 0x00 中断掩码寄存器
    Rx = 0x14,                   // RD n/a读 RX页
    Tx = 0x15,                   // WR n/a写 TX页
    RxCtrl = 0x16,               // WR n/a RX控制
    TxCtrl = 0x17,               // WR n/a TX控制
    RxAddr = 0x18,               // RD/WR 0x00 当前RX页读指针（极少用）
    RxPageFlag = 0x19,           // RD n/a当前RX页标记
    FilterM1 = 0x1a,             // RD/WR 0xff组播地址过滤器 1
    FilterM2 = 0x1b,             // RD/WR 0xff组播地址过滤器 2
    PllML = 0x30,                // RD/WR 0x12 PLLM[7:0](M:9bits)
    PllOdMH = 0x31,              // RD/WR 0x20 PLLOD 和 M[8]
    PllN = 0x32,                 // RD/WR 0x00 PLLN(5bits)
    PllCtrl = 0x33,              // RD/WR 0x01 PLL 控制
    PinIntCtrl = 0x34,           // RD/WR 0x00 INT 脚控制
    PinReCtrl = 0x35,            // RD/WR 0x00 RE 脚控制
    ClkStatus = 0x36,            // RD 0x01 时钟状态寄存器
};

struct Regset{
// 硬件版本寄存器 (地址：0x00) - 只读，存储芯片版本号
struct R8_HwVersion:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::HwVersion;
    uint8_t bits; // 版本号编码
};

// 时钟控制寄存器 (地址：0x01) - 读写，选择时钟源与软件复位
struct R8_ClockCtl:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::ClkCtrl;
    uint8_t clk_is_pll_else_osc:1; // 第 1 位：时钟源选择 [1=PLL 时钟/0=晶振时钟]
    uint8_t __resv1__:6;           // 第 2-7 位：预留位，配置为 0
    uint8_t sw_rst:1;              // 第 8 位：软件复位 [1=触发复位 (完成后自动清 0)/0=无复位]
};

// 功能配置寄存器 (地址：0x02) - 读写，配置 CDBUS 通信核心规则
struct R8_Setting:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::Setting;
    uint8_t enable_tote_pushpull:1;        // 第 1 位：TOE 引脚输出模式 [1=推挽/0=开漏]
    uint8_t invert_to:1;                   // 第 2 位：TO 引脚电平反转 [1=反转/0=正常]
    uint8_t enable_crc_handled_by_user:1;  // 第 3 位：CRC 校验处理 [1=软件处理/0=硬件处理]
    uint8_t enable_save_broken_data_frame:1; // 第 4 位：错误帧保存 [1=保存/0=丢弃]
    uint8_t cdbus_a_mode:1;                // 第 5 位：CDBUS A 模式 [1=启用/0=禁用]
    uint8_t cdbus_b_mode:1;                // 第 6 位：CDBUS B 模式 [1=启用/0=禁用]
    uint8_t full_duplex:1;                 // 第 7 位：全双工模式 [1=启用/0=半双工]
    uint8_t __resv1__:1;                   // 第 8 位：预留位，配置为 0
};

// 空闲等待长度寄存器 (地址：0x04) - 读写，配置总线空闲等待时长
struct R8_IdleWaitLen:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::IdleWaitLen;
    uint8_t bits; // 等待长度 (单位：时钟周期)
};

// 发送允许长度低位寄存器 (地址：0x05) - 读写，16 位发送允许时长低字节
struct R8_TxPermitLenL:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::TxPermitLenL;
    uint8_t bits; // 16 位数值低 8 位
};

// 发送允许长度高位寄存器 (地址：0x06) - 读写，16 位发送允许时长高字节
struct R8_TxPermitLenH:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::TxPermitLenH;
    uint8_t bits; // 16 位数值高 8 位
};

// 最大空闲长度低位寄存器 (地址：0x07) - 读写，16 位最大空闲时长低字节
struct R8_MaxIdleLenL:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::MaxIdleLenL;
    uint8_t bits; // 16 位数值低 8 位
};

// 最大空闲长度高位寄存器 (地址：0x08) - 读写，16 位最大空闲时长高字节
struct R8_MaxIdleLenH:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::MaxIdleLenH;
    uint8_t bits; // 16 位数值高 8 位
};

// 发送前导码长度寄存器 (地址：0x09) - 读写，配置发送数据包前导码长度
struct R8_TxPreamableLen:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::TxPreLen;
    uint8_t bits; // 前导码长度 (单位：比特)
};

// 地址过滤寄存器 (地址：0x0B) - 读写，配置地址过滤规则
struct R8_Filter:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::Filter;
    uint8_t filter_mode:4;   // 第 1-4 位：过滤模式选择 (精确匹配/掩码匹配等)
    uint8_t addr_match_en:4; // 第 5-8 位：地址匹配使能 [1=启用/0=禁用]
};

// 低速分频系数低位寄存器 (地址：0x0C) - 读写，16 位低速波特率分频系数低字节
struct R8_DivLsL:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::DivLsL;
    uint8_t bits; // 16 位分频系数低 8 位
};

// 低速分频系数高位寄存器 (地址：0x0D) - 读写，16 位低速波特率分频系数高字节
struct R8_DivLsH:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::DivLsH;
    uint8_t bits; // 16 位分频系数高 8 位
};

// 高速分频系数低位寄存器 (地址：0x0E) - 读写，16 位高速波特率分频系数低字节
struct R8_DivHsL:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::DivHsL;
    uint8_t bits; // 16 位分频系数低 8 位
};

// 高速分频系数高位寄存器 (地址：0x0F) - 读写，16 位高速波特率分频系数高字节
struct R8_DivHsH:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::DivHsH;
    uint8_t bits; // 16 位分频系数高 8 位
};

// 中断标志寄存器 (地址：0x10) - 只读，反馈中断触发类型
struct R8_IntFlag:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::IntFlag;
    uint8_t rx_complete:1;   // 第 1 位：接收完成中断 [1=触发/0=未触发]
    uint8_t tx_complete:1;   // 第 2 位：发送完成中断 [1=触发/0=未触发]
    uint8_t rx_error:1;      // 第 3 位：接收错误中断 (帧错误/CRC 错误) [1=触发/0=未触发]
    uint8_t bus_idle:1;      // 第 4 位：总线空闲中断 [1=触发/0=未触发]
    uint8_t __resv1__:4;     // 第 5-8 位：预留中断位，仅读取
};

// 中断掩码寄存器 (地址：0x11) - 读写，控制各中断类型的使能/禁用
struct R8_IntMask:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::IntMask;
    uint8_t rx_complete_en:1;  // 第 1 位：接收完成中断使能 [1=启用/0=禁用]
    uint8_t tx_complete_en:1;  // 第 2 位：发送完成中断使能 [1=启用/0=禁用]
    uint8_t rx_error_en:1;     // 第 3 位：接收错误中断使能 [1=启用/0=禁用]
    uint8_t bus_idle_en:1;     // 第 4 位：总线空闲中断使能 [1=启用/0=禁用]
    uint8_t __resv1__:4;       // 第 5-8 位：预留中断位，配置为 0
};

// 接收数据寄存器 (地址：0x14) - 只读，存储接收到的完整数据帧
struct R8_Rx:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::Rx;
    uint8_t data; // 接收数据 (需结合接收完成中断读取)
};

// 发送数据寄存器 (地址：0x15) - 只写，存储待发送的数据帧
struct R8_Tx:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::Tx;
    uint8_t data; // 发送数据 (写入后触发发送)
};

// 接收控制寄存器 (地址：0x16) - 读写，配置接收相关控制参数
struct R8_RxCtrl:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::RxCtrl;
    uint8_t rx_en:1;             // 第 1 位：接收使能 [1=启用/0=禁用]
    uint8_t rx_overflow_en:1;    // 第 2 位：接收溢出中断使能 [1=启用/0=禁用]
    uint8_t __resv1__:6;         // 第 3-8 位：预留位，配置为 0
};

// 发送控制寄存器 (地址：0x17) - 读写，配置发送相关控制参数
struct R8_TxCtrl:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::TxCtrl;
    uint8_t tx_en:1;             // 第 1 位：发送使能 [1=启用/0=禁用]
    uint8_t tx_overflow_en:1;    // 第 2 位：发送溢出中断使能 [1=启用/0=禁用]
    uint8_t __resv1__:6;         // 第 3-8 位：预留位，配置为 0
};

// 接收地址寄存器 (地址：0x18) - 读写，存储目标接收地址
struct R8_RxAddr:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::RxAddr;
    uint8_t addr; // 目标地址 (8 位地址编码)
};

// 接收页标记寄存器 (地址：0x19) - 只读，当前RX页标记
struct R8_RxPageFlag:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::RxPageFlag;
    uint8_t bits; // 页标记状态
};

// 组播地址过滤器 1 寄存器 (地址：0x1A) - 读写，组播地址过滤掩码 1
struct R8_FilterM1:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::FilterM1;
    uint8_t bits; // 过滤掩码值
};

// 组播地址过滤器 2 寄存器 (地址：0x1B) - 读写，组播地址过滤掩码 2
struct R8_FilterM2:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::FilterM2;
    uint8_t bits; // 过滤掩码值
};

// PLL M 低位寄存器 (地址：0x30) - 读写，PLLM[7:0]
struct R8_PllML:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::PllML;
    uint8_t pll_m_low; // PLLM[7:0] - PLL 倍频系数低 8 位
};

// PLL OD 和 M 高位寄存器 (地址：0x31) - 读写，PLLOD 和 PLLM[8]
struct R8_PllOdMH:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::PllOdMH;
    uint8_t pll_od:4;      // PLLOD[3:0] - PLL 输出分频系数
    uint8_t pll_m_high:1;  // PLLM[8] - PLL 倍频系数第 9 位
    uint8_t __resv1__:3;   // 第 5-8 位：预留位
};

// PLL N 寄存器 (地址：0x32) - 读写，PLLN(5bits)
struct R8_PllN:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::PllN;
    uint8_t pll_n:5;    // PLLN[4:0] - PLL 参考分频系数
    uint8_t __resv1__:3; // 第 6-8 位：预留位
};

// PLL 控制寄存器 (地址：0x33) - 读写，控制 PLL 启动、关闭与节能
struct R8_PllCtrl:public Reg8<>{ 
    static constexpr RegAddr REG_ADDR = RegAddr::PllCtrl;
    uint8_t pll_sleep_en:1; // 第 1 位：PLL 节能模式 [1=睡眠/0=正常工作]
    uint8_t __resv1__:3;    // 第 2-4 位：预留位
    uint8_t pll_on:1;       // 第 5 位：PLL 启动 [1=启动/0=关闭]
    uint8_t __resv2__:3;    // 第 6-8 位：预留位
};

// INT 引脚控制寄存器 (地址：0x34) - 读写，配置中断输出引脚电气特性
struct R8_IntCtrl:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::PinIntCtrl;
    uint8_t __resv1__:4;     // 第 1-4 位：预留位
    uint8_t is_pp_else_od:1; // 第 5 位：中断引脚输出模式 [1=推挽/0=开漏]
    uint8_t __resv2__:3;     // 第 6-8 位：预留位
};

// RE 引脚控制寄存器 (地址：0x35) - 读写，控制接收使能引脚状态与模式
struct R8_ReCtrl:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::PinReCtrl;
    uint8_t output_is_high:1; // 第 1 位：RE 引脚电平 [1=高电平/0=低电平]
    uint8_t __resv1__:2;      // 第 2-3 位：预留位
    uint8_t is_pp_else_od:1;  // 第 4 位：RE 引脚输出模式 [1=推挽/0=开漏]
    uint8_t __resv2__:4;      // 第 5-8 位：预留位
};

// 时钟状态寄存器 (地址：0x36) - 只读，反馈时钟系统工作状态
struct R8_ClockStatus:public Reg8<>{
    static constexpr RegAddr REG_ADDR = RegAddr::ClkStatus;
    uint8_t sysclk_source_is_osc:1; // 第 1 位：当前时钟源为晶振 [1=是/0=否]
    uint8_t sysclk_source_is_pll:1; // 第 2 位：当前时钟源为 PLL [1=是/0=否]
    uint8_t is_clock_switching:1;   // 第 3 位：时钟源切换中 [1=切换中/0=切换完成]
    uint8_t __resv1__:5;            // 第 4-8 位：预留位，仅读取
};

};
};



}