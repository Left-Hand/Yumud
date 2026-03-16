
#pragma once

// 适用于GD32E503xx、GD32E505xx、GD32E507xx、GD32E508xx、
// GD32EPRTxx


#include "sxx32_common_can_def.hpp"

namespace ymd::ral::can::gd32e50x_specified{


// CAN基地址: CAN0=0x40006400 CAN1=0x40006800 CAN2=0x4000CC00
// 26.4.1 控制寄存器(CAN_CTL) [0x00] 复位值:0x00010002 仅32位访问
// struct [[nodiscard]] R32_CAN_CTL {
//     uint32_t IWMOD:1;        // 初始化工作模式:0-禁用 1-使能 rw 复位值(1)
//     uint32_t SLPWMOD:1;      // 睡眠工作模式:0-正常 1-睡眠 rw 复位值(0)
//     uint32_t TFO:1;          // 发送FIFO溢出:0-未溢出 1-溢出 rw 复位值(0)
//     uint32_t RFOD:1;         // 接收FIFO满覆盖:0-使能覆盖 1-禁用覆盖 rw 复位值(0)
//     uint32_t ARD:1;          // 自动重发:0-使能 1-禁用 rw 复位值(0)
//     uint32_t AWU:1;          // 自动唤醒:0-软件 1-硬件 rw 复位值(0)
//     uint32_t ABOR:1;         // 自动离线恢复:1-硬件自动恢复 rw 复位值(0)
//     uint32_t TTC:1;          // 时间触发通信:0-禁用 1-使能 rw 复位值(0)
//     uint32_t :7;    // 保留位 必须保持复位值 - 复位值(0)
//     uint32_t SWRST:1;        // 软件复位:1-复位CAN并睡眠 硬件自动清0 rs 复位值(0)
//     uint32_t DFZ:1;          // 调试冻结:DBG_HOLD置位时生效 0-正常 1-停止 rw 复位值(0)
//     uint32_t :15;   // 保留位 必须保持复位值 - 复位值(0)
// };

using R32_CAN_CTL = sxx32_common::R32_CAN_CTLR;

// 26.4.2 状态寄存器(CAN_STAT) [0x04] 复位值:0x00000C02 仅32位访问
// struct [[nodiscard]] R32_CAN_STAT {
//     uint32_t IWS:1;          // 初始化工作状态:0-未初始化 1-初始化中 r 复位值(1)
//     uint32_t SLPWS:1;        // 睡眠工作状态:0-未睡眠 1-睡眠中 r 复位值(0)
//     uint32_t ERRIF:1;        // 错误中断标志:1-有错误 软件写1清零 rc_w1 复位值(0)
//     uint32_t :3;    // 保留位 必须保持复位值 - 复位值(0)
//     uint32_t WUIF:1;         // 唤醒中断标志:1-有唤醒 软件写1清零 rc_w1 复位值(0)
//     uint32_t SLPIF:1;        // 睡眠中断标志:1-有睡眠 软件写1清零 rc_w1 复位值(0)
//     uint32_t :2;    // 保留位 必须保持复位值 - 复位值(0)
//     uint32_t TS:1;           // 发送状态:0-非发送器 1-发送器 r 复位值(0)
//     uint32_t RS:1;           // 接收状态:0-非接收器 1-接收器 r 复位值(0)
//     uint32_t LASTRX:1;       // RX引脚最近一次采样值 r 复位值(x)
//     uint32_t RXL:1;          // RX引脚当前电平 r 复位值(x)
//     uint32_t :20;   // 保留位 必须保持复位值 - 复位值(0)
// };

using R32_CAN_STAT = sxx32_common::R32_CAN_STATR;

// 26.4.3 发送状态寄存器(CAN_TSTAT) [0x08] 复位值:0x1C000000 仅32位访问


// struct [[nodiscard]] R32_CAN_TSTAT {
//     uint32_t MTF0:1;         // 邮箱0发送完成:1-完成/中止 软件写1清零 rc_w1 复位值(0)
//     uint32_t MTFNERR0:1;     // 邮箱0无错发送:1-无错完成 软件写1清零 rc_w1 复位值(0)
//     uint32_t MAL0:1;         // 邮箱0仲裁失败:1-失败 软件写1清零 rc_w1 复位值(0)
//     uint32_t MTE0:1;         // 邮箱0发送错误:1-错误 软件写1清零 rc_w1 复位值(0)
//     uint32_t :3;    // 保留位 必须保持复位值 - 复位值(0)
//     uint32_t MST0:1;         // 邮箱0停止发送:1-停止 邮箱空时硬件清0 rs 复位值(0)
//     uint32_t MTF1:1;         // 邮箱1发送完成:1-完成/中止 软件写1清零 rc_w1 复位值(0)
//     uint32_t MTFNERR1:1;     // 邮箱1无错发送:1-无错完成 软件写1清零 rc_w1 复位值(0)
//     uint32_t MAL1:1;         // 邮箱1仲裁失败:1-失败 软件写1清零 rc_w1 复位值(0)
//     uint32_t MTE1:1;         // 邮箱1发送错误:1-错误 软件写1清零 rc_w1 复位值(0)
//     uint32_t :3;    // 保留位 必须保持复位值 - 复位值(0)
//     uint32_t MST1:1;         // 邮箱1停止发送:1-停止 邮箱空时硬件清0 rs 复位值(0)
//     uint32_t MTF2:1;         // 邮箱2发送完成:1-完成/中止 软件写1清零 rc_w1 复位值(0)
//     uint32_t MTFNERR2:1;     // 邮箱2无错发送:1-无错完成 软件写1清零 rc_w1 复位值(0)
//     uint32_t MAL2:1;         // 邮箱2仲裁失败:1-失败 软件写1清零 rc_w1 复位值(0)
//     uint32_t MTE2:1;         // 邮箱2发送错误:1-错误 软件写1清零 rc_w1 复位值(0)
//     uint32_t :3;    // 保留位 必须保持复位值 - 复位值(0)
//     uint32_t MST2:1;         // 邮箱2停止发送:1-停止 邮箱空时硬件清0 rs 复位值(0)
//     uint32_t NUM:2;          // 发送邮箱号:FIFO不满=下一个 FIFO满=最后一个 r 复位值(x)
//     uint32_t TME0:1;         // 邮箱0空:1-空 0-非空 r 复位值(1)
//     uint32_t TME1:1;         // 邮箱1空:1-空 0-非空 r 复位值(1)
//     uint32_t TME2:1;         // 邮箱2空:1-空 0-非空 r 复位值(1)
//     uint32_t TMLS0:1;        // 邮箱0最后发送:1-多帧时最后发送 r 复位值(0)
//     uint32_t TMLS1:1;        // 邮箱1最后发送:1-多帧时最后发送 r 复位值(0)
//     uint32_t TMLS2:1;        // 邮箱2最后发送:1-多帧时最后发送 r 复位值(0)
// };

using R32_CAN_TSTAT = sxx32_common::R32_CAN_TSTATR;

// 26.4.4 接收FIFO0寄存器(CAN_RFIFO0) [0x0C] 复位值:0x00000000 仅32位访问
// struct [[nodiscard]] R32_CAN_RFIFO0 {
//     uint32_t RFL0:2;         // 接收FIFO0帧数量:0-3帧 r 复位值(0)
//     uint32_t :1;    // 保留位 必须保持复位值 - 复位值(0)
//     uint32_t RFF0:1;         // 接收FIFO0满:1-满 软件写1清零 rc_w1 复位值(0)
//     uint32_t RFO0:1;         // 接收FIFO0溢出:1-溢出 软件写1清零 rc_w1 复位值(0)
//     uint32_t RFD0:1;         // 释放FIFO0数据:1-释放 硬件自动清0 rs 复位值(0)
//     uint32_t :26;   // 保留位 必须保持复位值 - 复位值(0)
// };

using R32_CAN_RFIFO = sxx32_common::R32_CAN_RFIFO;


// 26.4.6 中断使能寄存器(CAN_INTEN) [0x14] 复位值:0x00000000 仅32位访问


// struct [[nodiscard]] R32_CAN_INTEN {
//     uint32_t TMEIE:1;        // 发送邮箱空中断:0-禁用 1-使能 rw 复位值(0)
//     uint32_t RFNEIE0:1;      // FIFO0非空中断:0-禁用 1-使能 rw 复位值(0)
//     uint32_t RFFIE0:1;       // FIFO0满中断:0-禁用 1-使能 rw 复位值(0)
//     uint32_t RFOIE0:1;       // FIFO0溢出中断:0-禁用 1-使能 rw 复位值(0)
//     uint32_t RFNEIE1:1;      // FIFO1非空中断:0-禁用 1-使能 rw 复位值(0)
//     uint32_t RFFIE1:1;       // FIFO1满中断:0-禁用 1-使能 rw 复位值(0)
//     uint32_t RFOIE1:1;       // FIFO1溢出中断:0-禁用 1-使能 rw 复位值(0)
//     uint32_t :1;    // 保留位 必须保持复位值 - 复位值(0)
//     uint32_t WERRIE:1;       // 警告错误中断:0-禁用 1-使能 rw 复位值(0)
//     uint32_t PERRIE:1;       // 被动错误中断:0-禁用 1-使能 rw 复位值(0)
//     uint32_t BOIE:1;         // 离线错误中断:0-禁用 1-使能 rw 复位值(0)
//     uint32_t ERRNIE:1;       // 错误种类中断:0-禁用 1-使能 rw 复位值(0)
//     uint32_t :3;    // 保留位 必须保持复位值 - 复位值(0)
//     uint32_t ERRIE:1;        // 总错误中断:0-禁用 1-使能 rw 复位值(0)
//     uint32_t WIE:1;          // 唤醒中断:0-禁用 1-使能 rw 复位值(0)
//     uint32_t SLPWIE:1;       // 睡眠中断:0-禁用 1-使能 rw 复位值(0)
//     uint32_t :14;   // 保留位 必须保持复位值 - 复位值(0)
// };

using R32_CAN_INTEN = sxx32_common::R32_CAN_INTEN;

// 26.4.7 错误寄存器(CAN_ERR) [0x18] 复位值:0x00000000 仅32位访问
// struct [[nodiscard]] R32_CAN_ERR {
//     uint32_t WERR:1;         // 警告错误:1-TEC/REC≥96 r 复位值(0)
//     uint32_t PERR:1;         // 被动错误:1-TEC/REC>127 r 复位值(0)
//     uint32_t BOERR:1;        // 离线错误:1-TEC溢出(>255) r 复位值(0)
//     uint32_t :1;    // 保留位 必须保持复位值 - 复位值(0)
//     uint32_t ERRN:3;         // 错误种类:000-无 001-填充 010-格式 011-ACK 100-隐性 101-显性 110-CRC 111-软件 rw 复位值(000)
//     uint32_t :9;    // 保留位 必须保持复位值 - 复位值(0)
//     uint32_t TECNT:8;        // 发送错误计数值:0-255 r 复位值(0)
//     uint32_t RECNT:8;        // 接收错误计数值:0-255 r 复位值(0)
// };

using R32_CAN_ERR = sxx32_common::R32_CAN_ERRSR;

// 26.4.8 位时序寄存器(CAN_BT) [0x1C] 复位值:0x01230000 仅32位访问
struct [[nodiscard]] R32_CAN_BT {
    uint32_t BAUDPSC:10;     // 波特率分频系数:0-1023 rw 复位值(0)
    uint32_t BS1_6_4:3;          // 位段1时间单元:BS1+1 10-12位为GD32E508xx扩展位 rw 复位值(x)
    uint32_t BS2_4_3:2;          // 位段2时间单元:BS2+1 13-14位为GD32E508xx扩展位 rw 复位值(x)
    uint32_t :1;
    uint32_t BS1:4;
    uint32_t BS2:3;
    uint32_t :1;
    uint32_t SWJ:5;
    uint32_t :1;    // 保留位 必须保持复位值 - 复位值(0)
    uint32_t LCMOD:1;        // 回环模式:0-禁用 1-使能 rw 复位值(0)
    uint32_t SCMOD:1;        // 静默模式:0-禁用 1-使能 rw 复位值(0)
};

VALIDATE_R32(R32_CAN_BT)


// 26.4.9 FD控制寄存器(CAN_FDCTL) [0x20] 复位值:0x00000000 仅GD32E508xx有效 仅32位访问
struct [[nodiscard]] R32_CAN_FDCTL {
    uint32_t FDEN:1;         // FD功能使能:0-禁用 1-使能 rw 复位值(0)
    uint32_t :1;    // 保留位 必须保持复位值 - 复位值(0)
    uint32_t PRED:1;         // 协议异常检测:0-使能 1-禁用 rw 复位值(0)
    uint32_t NSIO:1;         // 协议标准:0-ISO 1-BOSCH rw 复位值(0)
    uint32_t TDCEN:1;        // 传输延迟补偿:0-禁用 1-使能 rw 复位值(0)
    uint32_t TDCMOD:1;       // 延迟补偿模式:0-测量+偏移 1-仅偏移 rw 复位值(0)
    uint32_t ESIMOD:1;       // 错误状态指示:1-由TMPx.ESI决定 rw 复位值(0)
    uint32_t :25;   // 保留位 必须保持复位值 - 复位值(0)
};

VALIDATE_R32(R32_CAN_FDCTL)

// 26.4.10 FD状态寄存器(CAN_FDSTAT) [0x24] 复位值:0x00000000 仅GD32E508xx有效 仅32位访问
struct [[nodiscard]] R32_CAN_FDSTAT {
    uint32_t TDCV:7;         // 传输延迟补偿值:硬件自动计算 r 复位值(0)
    uint32_t :8;    // 保留位 必须保持复位值 - 复位值(0)
    uint32_t PRE:1;          // 协议异常事件:1-检测到 软件写1清零 rc_w1 复位值(0)
    uint32_t :16;   // 保留位 必须保持复位值 - 复位值(0)
};

VALIDATE_R32(R32_CAN_FDSTAT)

// 26.4.11 FD传输延迟补偿寄存器(CAN_FDTDC) [0x28] 复位值:0x00000000 仅GD32E508xx有效 仅32位访问
struct [[nodiscard]] R32_CAN_FDTDC {
    uint32_t TDCF:7;         // 延迟补偿过滤器:次级采样点最小值 rw 复位值(0)
    uint32_t :1;    // 保留位 必须保持复位值 - 复位值(0)
    uint32_t TDCO:7;         // 延迟补偿偏移:次级采样点偏移量 rw 复位值(0)
    uint32_t :17;   // 保留位 必须保持复位值 - 复位值(0)
};

VALIDATE_R32(R32_CAN_FDTDC)

// 26.4.12 数据位时序寄存器(CAN_DBT) [0x2C] 复位值:0x01230000 仅GD32E508xx有效 仅32位访问
struct [[nodiscard]] R32_CAN_DBT {
    uint32_t DBAUDPSC:10;    // 数据位波特率分频:0-1023 rw 复位值(0)
    uint32_t :6;    // 保留位 必须保持复位值 - 复位值(0)
    uint32_t DBS1:4;         // 数据位段1时间单元:DBS1+1 rw 复位值(x)
    uint32_t DBS2:3;         // 数据位段2时间单元:DBS2+1 rw 复位值(x)
    uint32_t :1;    // 保留位 必须保持复位值 - 复位值(0)
    uint32_t DSJW:3;         // 数据位再同步宽度:DSJW+1 rw 复位值(x)
    uint32_t :5;    // 保留位 必须保持复位值 - 复位值(0)
};

VALIDATE_R32(R32_CAN_DBT)


// 26.4.13 发送邮箱标识符寄存器(CAN_TMIx) [0x180+0x10*x] x=0-2 复位值:0xXXXXXXX0 仅32位访问
struct [[nodiscard]] R32_CAN_TMIx {
    uint32_t TEN:1;          // 发送使能:1-启动发送 硬件自动清0 rw 复位值(0)
    uint32_t FT:1;           // 帧种类:0-数据帧 1-遥控帧 rw 复位值(0)
    uint32_t FF:1;           // 帧格式:0-标准帧 1-扩展帧 rw 复位值(0)
    uint32_t EFID12_0:13;    // 扩展帧ID[12:0] rw 复位值(x)
    uint32_t EFID17_13:5;    // 扩展帧ID[17:13] rw 复位值(x)
    uint32_t SFID_EFID:11;   // 标准帧ID[10:0]/扩展帧ID[28:18] rw 复位值(x)
};

VALIDATE_R32(R32_CAN_TMIx)

// 26.4.14 发送邮箱属性寄存器(CAN_TMPx) [0x184+0x10*x] x=0-2 复位值:0xXXXXXXXX 仅32位访问
struct [[nodiscard]] R32_CAN_TMPx {
    uint32_t DLENC:4;        // 数据长度:0-8/64(GD32E508xx) 字节 rw 复位值(0)
    uint32_t ESI:1;          // 错误状态指示:GD32E508xx有效 rw 复位值(0)
    uint32_t BRS:1;          // 位速率转换:GD32E508xx有效 rw 复位值(0)
    uint32_t :1;    // 保留位 必须保持复位值 - 复位值(0)
    uint32_t FDF:1;          // CAN-FD帧标志:1-FD帧 GD32E508xx有效 rw 复位值(0)
    uint32_t TSEN:1;         // 时间戳使能:1-使能 存入TMDATA1 rw 复位值(0)
    uint32_t :7;    // 保留位 必须保持复位值 - 复位值(0)
    uint32_t TS:16;          // 发送时间戳 rw 复位值(0)
};

VALIDATE_R32(R32_CAN_TMPx)


// 26.4.17 接收FIFO邮箱标识符寄存器(CAN_RFIFOMIx) [0x1B0+0x10*x] x=0-1 复位值:0xXXXXXXXX 仅32位访问
struct [[nodiscard]] R32_CAN_RFIFOMIx {
    uint32_t :1;    // 保留位 必须保持复位值 - 复位值(0)
    uint32_t FT:1;           // 帧种类:0-数据帧 1-遥控帧 r 复位值(0)
    uint32_t FF:1;           // 帧格式:0-标准帧 1-扩展帧 r 复位值(0)
    uint32_t ID:29;
};

VALIDATE_R32(R32_CAN_RFIFOMIx)


// 26.4.18 接收FIFO邮箱属性寄存器(CAN_RFIFOMPx) [0x1B4+0x10*x] x=0-1 复位值:0xXXXXXXXX 仅32位访问
struct [[nodiscard]] R32_CAN_RFIFOMPx {
    uint32_t DLENC:4;        // 数据长度:0-8/64(GD32E508xx) 字节 r 复位值(0)
    uint32_t ESI:1;          // 错误状态指示:GD32E508xx有效 r 复位值(0)
    uint32_t BRS:1;          // 位速率转换:GD32E508xx有效 r 复位值(0)
    uint32_t :1;    // 保留位 必须保持复位值 - 复位值(0)
    uint32_t FDF:1;          // CAN-FD帧标志:1-FD帧 GD32E508xx有效 r 复位值(0)
    uint32_t FI:8;           // 过滤索引:帧通过的过滤器序号 r 复位值(0)
    uint32_t TS:16;          // 接收时间戳 r 复位值(0)
};

VALIDATE_R32(R32_CAN_RFIFOMPx)



// CAN外设寄存器整体映射
struct [[nodiscard]] CAN_Def {
    volatile sxx32_common::R32_CAN_CTLR CTLR;
    volatile sxx32_common::R32_CAN_STATR STATR;
    volatile sxx32_common::R32_CAN_TSTATR TSTATR;
    volatile sxx32_common::R32_CAN_RFIFO RFIFO[2];
    volatile sxx32_common::R32_CAN_INTEN INTENR;
    volatile sxx32_common::R32_CAN_ERRSR ERRSR;

    volatile R32_CAN_BT BT;               // 0x1C 位时序寄存器
    volatile R32_CAN_FDCTL FDCTL;         // 0x20 FD控制寄存器(GD32E508xx)
    volatile R32_CAN_FDSTAT FDSTAT;       // 0x24 FD状态寄存器(GD32E508xx)
    volatile R32_CAN_FDTDC FDTDC;         // 0x28 FD延迟补偿寄存器(GD32E508xx)
    volatile R32_CAN_DBT DBT;             // 0x2C 数据位时序寄存器(GD32E508xx)
    volatile uint32_t RESERVED1[84];  // 0x30-0x17F 保留地址

    struct [[nodiscard]] TxMailBox{
        volatile R32_CAN_TMIx TMI;         // 0x180/0x190/0x1A0 发送邮箱标识符x0-x2
        volatile R32_CAN_TMPx TMP;         // 0x184/0x194/0x1A4 发送邮箱属性x0-x2
        volatile uint32_t TXMDLR; 
        volatile uint32_t TXMDHR; 
    };

    struct [[nodiscard]] FifoMailBox{
        volatile R32_CAN_RFIFOMIx  RXMIR;
        volatile R32_CAN_RFIFOMPx RXMDTR;
        volatile uint32_t RXMDLR;
        volatile uint32_t RXMDHR;
    };

    struct [[nodiscard]] CAN_Filter_Pair{
        // R32_CAN_FiR FIR[2];
        volatile uint32_t FR1;
        volatile uint32_t FR2;
    };

    TxMailBox sTxMailBox[3];
    FifoMailBox sFifoMailBox[3];

};




static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CTLR) == 0);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::STATR) == 4);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::TSTATR) == 8);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::RFIFO[0]) == 12);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::RFIFO[1]) == 16);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::INTENR) == 20);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::ERRSR) == 24);

static_assert(__builtin_offsetof(CAN_Def, CAN_Def::BT) == 0x1c);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::FDCTL) == 0x20);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::FDSTAT) == 0x24);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::FDTDC) == 0x28);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::DBT) == 0x2c);

static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sTxMailBox[0]) == 0x180);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sTxMailBox[1]) == 0x190);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sTxMailBox[2]) == 0x1a0);

static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sFifoMailBox[0]) == 0x1B0);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sFifoMailBox[1]) == 0x1c0);

using CAN_Filter_Def = sxx32_common::CAN_Filter_Def;



}