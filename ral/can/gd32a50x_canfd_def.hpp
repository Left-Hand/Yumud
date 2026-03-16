#pragma once

#include "sxx32_common_can_regs.hpp"

namespace ymd::ral::can::gd32a50x{

// 23.5.1 控制寄存器0(CAN_CTL0) [0x00] 复位值:0x5900000F 32位
// 配置约束：除位30/28/25/19外，其余位仅暂停模式可配；除位31/27/24/20外，其余位可被SWRST复位；仅32位字访问
struct [[nodiscard]] R32_CAN_CTL0 {
    uint32_t MSZ:5;         // 4:0 内存大小 rw 0x0F
    uint32_t :3;        // 7:5 保留位 - -
    uint32_t FS:2;          // 9:8 格式选择 rw -
    uint32_t :1;        // 10 保留位 - -
    uint32_t FDEN:1;        // 11 CAN FD模式使能 rw -
    uint32_t MST:1;         // 12 邮箱中止发送 rw -
    uint32_t LAPRIOEN:1;    // 13 本地仲裁优先级使能 rw -
    uint32_t PNMOD:1;       // 14 虚拟联网模式选择 rw -
    uint32_t DMAEN:1;       // 15 DMA使能 rw -
    uint32_t RPFQEN:1;      // 16 接收私有过滤+邮箱队列使能 rw -
    uint32_t SRDIS:1;       // 17 自接收禁能 rw -
    uint32_t PNS:1;         // 18 虚拟联网状态 rw -
    uint32_t PNEN:1;        // 19 虚拟联网模式使能 rw -（仅暂停模式可配）
    uint32_t LPS:1;         // 20 低功耗状态 r -
    uint32_t WERREN:1;      // 21 错误警告使能 rw -
    uint32_t :2;        // 23:22 保留位 - -
    uint32_t INAS:1;        // 24 暂停模式状态 r -
    uint32_t SWRST:1;       // 25 软件复位 rw -（仅暂停模式可配）
    uint32_t :1;        // 26 保留位 - -
    uint32_t NRDY:1;        // 27 未准备好 r -
    uint32_t HALT:1;        // 28 暂停CAN rw -（仅暂停模式可配）
    uint32_t RFEN:1;        // 29 Rx FIFO使能 rw -
    uint32_t INAMOD:1;      // 30 暂停模式使能 rw -（仅暂停模式可配）
    uint32_t CANDIS:1;      // 31 CAN禁能 rw -
};

// 23.5.2 控制寄存器1(CAN_CTL1) [0x04] 复位值:0x00000000 32位
// 配置约束：位12/7/5/4/3仅暂停模式可配；所有位不被SWRST复位；仅32位字访问
struct [[nodiscard]] R32_CAN_CTL1 {
    uint32_t :3;        // 2:0 保留位 - -
    uint32_t MMOD:1;        // 3 监听模式 rw 0（仅暂停模式可配）
    uint32_t MTO:1;         // 4 邮箱发送顺序 rw 0（仅暂停模式可配）
    uint32_t TSYNC:1;       // 5 时间同步使能 rw 0（仅暂停模式可配）
    uint32_t ABORDIS:1;     // 6 自动离线恢复不使能 rw 0
    uint32_t BSPMOD:1;      // 7 位采样模式 rw 0（仅暂停模式可配）
    uint32_t :2;        // 9:8 保留位 - -
    uint32_t RWERRIE:1;     // 10 接收错误警告中断使能 rw 0（仅WERREN=1可写）
    uint32_t TWERRIE:1;     // 11 发送错误警告中断使能 rw 0（仅WERREN=1可写）
    uint32_t LSCMOD:1;      // 12 回环静默模式 rw 0（仅暂停模式可配）
    uint32_t :1;        // 13 保留位 - -
    uint32_t ERRSIE:1;      // 14 错误汇总中断使能 rw 0
    uint32_t BOIE:1;        // 15 离线中断使能 rw 0
    uint32_t :16;       // 31:16 保留位 - -
};

// 23.5.3 计数器寄存器(CAN_TIMER) [0x08] 复位值:0x00000000 32位
// 配置约束：仅32位字访问
struct [[nodiscard]] R32_CAN_TIMER {
    uint32_t CNT:16;        // 15:0 计数器值 rw 0x0000
    uint32_t :16;       // 31:16 保留位 - -
};

// 23.5.4 接收邮箱公有过滤寄存器(CAN_RMPUBF) [0x10] 复位值:0xXXXXXXXX 32位
// 配置约束：位于RAM；所有位仅暂停模式可配；仅32位字访问
struct [[nodiscard]] R32_CAN_RMPUBF {
    uint32_t MFD:29;        // 28:0 邮箱过滤数据（ID域） rw X
    uint32_t MFD29:1;       // 29 保留 rw X
    uint32_t MFD30:1;       // 30 过滤IDE域 rw X
    uint32_t MFD31:1;       // 31 过滤RTR域 rw X
};

// 23.5.5 错误寄存器0(CAN_ERR0) [0x1C] 复位值:0x00000000 32位
// 配置约束：除暂停模式外所有位只读；仅32位字访问
struct [[nodiscard]] R32_CAN_ERR0 {
    uint32_t TECNT:8;       // 7:0 发送错误计数器 rw0 0x00（暂停模式仅可写0）
    uint32_t RECNT:8;       // 15:8 接收错误计数器 rw0 0x00（暂停模式仅可写0）
    uint32_t TEFCNT:8;      // 23:16 FD帧数据阶段发送错误计数器 rw0 0x00（暂停模式仅可写0）
    uint32_t REFCNT:8;      // 31:24 FD帧数据阶段接收错误计数器 rw0 0x00（暂停模式仅可写0）
};

// 23.5.6 错误寄存器1(CAN_ERR1) [0x20] 复位值:0x000X000X 32位
// 配置约束：仅32位字访问
struct [[nodiscard]] R32_CAN_ERR1 {
    uint32_t :1;        // 0 保留位 - -
    uint32_t ERRSF:1;       // 1 错误汇总标志 rc_w1 X（写1清零）
    uint32_t BOF:1;         // 2 离线标志 rc_w1 X（写1清零）
    uint32_t RS:1;          // 3 接收状态 r X
    uint32_t ERRSI:2;       // 5:4 错误状态指示 r X
    uint32_t TS:1;          // 6 发送状态 r X
    uint32_t IDLEF:1;       // 7 空闲标志 r X
    uint32_t RWERRF:1;      // 8 接收错误警告标志 rc X
    uint32_t TWERRF:1;      // 9 发送错误警告标志 rc X
    uint32_t STFERR:1;      // 10 填充错误 rc_w1 X（写1清零）
    uint32_t FMERR:1;       // 11 格式错误 rc_w1 X（写1清零）
    uint32_t CRCERR:1;      // 12 CRC错误 rc_w1 X（写1清零）
    uint32_t ACKERR:1;      // 13 ACK错误 rc_w1 X（写1清零）
    uint32_t BDERR:1;       // 14 位显性错误 rc_w1 X（写1清零）
    uint32_t BRERR:1;       // 15 位隐性错误 rc_w1 X（写1清零）
    uint32_t RWERRIF:1;     // 16 接收错误警告中断标志 rc_w1 X（写1清零）
    uint32_t TWERRIF:1;     // 17 发送错误警告中断标志 rc_w1 X（写1清零）
    uint32_t SYN:1;         // 18 同步标志 r X
    uint32_t BORF:1;        // 19 离线恢复标志 rc_w1 X（写1清零）
    uint32_t ERRFSF:1;      // 20 FD帧数据阶段错误汇总标志 rc_w1 X（写1清零）
    uint32_t ERROVR:1;      // 21 错误溢出 rc_w1 X（写1清零）
    uint32_t :4;        // 25:22 保留位 - -
    uint32_t STFFERR:1;     // 26 FD帧数据阶段填充错误 rc X
    uint32_t FMFERR:1;      // 27 FD帧数据阶段格式错误 rc X
    uint32_t CRCFERR:1;     // 28 FD帧数据阶段CRC错误 rc X
    uint32_t :1;        // 29 保留位 - -
    uint32_t BDFERR:1;      // 30 FD帧数据阶段位显性错误 rc X
    uint32_t BRFERR:1;      // 31 FD帧数据阶段位隐性错误 rc X
};

// 23.5.7 中断使能寄存器(CAN_INTEN) [0x28] 复位值:0x00000000 32位
// 配置约束：仅32位字访问
struct [[nodiscard]] R32_CAN_INTEN {
    uint32_t MIEx:32;       // 31:0 消息收发中断使能 rw 0x00000000（MIE5-7对应Rx FIFO）
};

// 23.5.8 状态寄存器(CAN_STAT) [0x30] 复位值:0x00000000 32位
// 配置约束：RFEN配置改变时位1-7清零；仅32位字访问；所有标志位写1清零
struct [[nodiscard]] R32_CAN_STAT {
    uint32_t MS0_RFC:1;     // 0 邮箱0状态/清Rx FIFO rc_w1 0
    uint32_t MS1_RES:1;     // 1 邮箱1状态/保留 rc_w1 0
    uint32_t MS2_RES:1;     // 2 邮箱2状态/保留 rc_w1 0
    uint32_t MS3_RES:1;     // 3 邮箱3状态/保留 rc_w1 0
    uint32_t MS4_RES:1;     // 4 邮箱4状态/保留 rc_w1 0
    uint32_t MS5_RFNE:1;    // 5 邮箱5状态/Rx FIFO非空 rc_w1 0
    uint32_t MS6_RFW:1;     // 6 邮箱6状态/Rx FIFO警告 rc_w1 0
    uint32_t MS7_RFO:1;     // 7 邮箱7状态/Rx FIFO溢出 rc_w1 0
    uint32_t MSx:24;        // 31:8 邮箱8-31状态 rc_w1 0
};

// 23.5.9 控制寄存器2(CAN_CTL2) [0x34] 复位值:0x00A00000 32位
// 配置约束：除位31/30外，其余位仅暂停模式可配；所有位不被SWRST复位；仅32位字访问
struct [[nodiscard]] R32_CAN_CTL2 {
    uint32_t :11;       // 10:0 保留位 - -
    uint32_t EFDIS:1;       // 11 边沿过滤禁能 rw 0
    uint32_t ISO:1;         // 12 ISO CAN FD rw 0
    uint32_t :1;        // 13 保留位 - -
    uint32_t PREEN:1;       // 14 协议异常检测使能 rw 0
    uint32_t ITSRC:1;       // 15 内部计数器时钟源 rw 0
    uint32_t IDERTR_RMF:1;  // 16 IDE/RTR域过滤类型 rw 0
    uint32_t RRFRMS:1;      // 17 远程请求帧存储 rw 0
    uint32_t RFO:1;         // 18 接收过滤顺序 rw 0
    uint32_t ASD:5;         // 23:19 仲裁启动延迟 rw 0
    uint32_t RFFN:4;        // 27:24 Rx FIFO过滤器数目 rw 0xA
    uint32_t :2;        // 29:28 保留位 - -
    uint32_t BORIE:1;       // 30 离线恢复中断使能 rw 0
    uint32_t ERRFSIE:1;     // 31 FD帧数据阶段错误汇总中断使能 rw 0
};

// 23.5.10 常规帧CRC寄存器(CAN_CRCC) [0x44] 复位值:0x00000000 32位
// 配置约束：仅32位字访问
struct [[nodiscard]] R32_CAN_CRCC {
    uint32_t CRCTC:15;      // 14:0 常规帧CRC计算值 r 0x00000
    uint32_t :1;        // 15 保留位 - -
    uint32_t ANTM:5;        // 20:16 关联邮箱编号 r 0x00
    uint32_t :11;       // 31:21 保留位 - -
};

// 23.5.11 接收FIFO共有过滤寄存器(CAN_RFIFOPUBF) [0x48] 复位值:0xXXXXXXXX 32位
// 配置约束：位于RAM；所有位仅暂停模式可配；仅32位字访问
struct [[nodiscard]] R32_CAN_RFIFOPUBF {
    uint32_t FFDx:32;       // 31:0 Rx FIFO过滤数据 rw X
};

// 23.5.12 接收FIFO标识符过滤元素匹配序号寄存器(CAN_RFIFOIFMN) [0x4C] 复位值:0xXXXXXXXX 32位
// 配置约束：仅32位字访问
struct [[nodiscard]] R32_CAN_RFIFOIFMN {
    uint32_t IDFMN:9;       // 8:0 匹配序号 r X
    uint32_t :23;       // 31:9 保留位 - -
};

// 23.5.13 位时间寄存器(CAN_BT) [0x50] 复位值:0x01000000 32位
// 配置约束：所有位仅暂停模式可配；所有位不被SWRST复位；仅32位字访问
struct [[nodiscard]] R32_CAN_BT {
    uint32_t PBS2:5;        // 4:0 相位缓冲段2 rw 0x00
    uint32_t PBS1:5;        // 9:5 相位缓冲段1 rw 0x00
    uint32_t PTS:6;         // 15:10 传播时间段 rw 0x00
    uint32_t SJW:5;         // 20:16 再同步补偿宽度 rw 0x00
    uint32_t BAUDPSC:10;    // 30:21 波特率分频系数 rw 0x0100
    uint32_t :1;        // 31 保留位 - -
};

// 23.5.14 接收FIFO/邮箱私有过滤x寄存器(CAN_RFIFOMPFx) [0x880+4*x] 复位值:0xXXXXXXXX 32位
// 配置约束：位于RAM；所有位仅暂停模式可配；所有位不被SWRST复位；仅32位字访问
struct [[nodiscard]] R32_CAN_RFIFOMPFx {
    uint32_t FMFDx:32;      // 31:0 FIFO/邮箱过滤数据 rw X
};

// 23.5.15 虚拟联网模式控制寄存器0(CAN_PN_CTL0) [0xB00] 复位值:0x00000100 32位
// 配置约束：除位17/16外，其余位仅暂停模式可配；仅32位字访问
struct [[nodiscard]] R32_CAN_PN_CTL0 {
    uint32_t FFT:2;         // 1:0 帧过滤类型 rw 0x00
    uint32_t IDFT:2;        // 3:2 ID域过滤类型 rw 0x00
    uint32_t DATAFT:2;      // 5:4 DATA域过滤类型 rw 0x00
    uint32_t :2;        // 7:6 保留位 - -
    uint32_t NMM:8;         // 15:8 消息匹配次数 rw 0x01
    uint32_t WMIE:1;        // 16 匹配唤醒中断使能 rw 0x00
    uint32_t WTOIE:1;       // 17 超时唤醒中断使能 rw 0x00
    uint32_t :14;       // 31:18 保留位 - -
};

// 23.5.16 虚拟联网模式超时寄存器(CAN_PN_TO) [0xB04] 复位值:0x00000000 32位
// 配置约束：所有位仅暂停模式可配；仅32位字访问
struct [[nodiscard]] R32_CAN_PN_TO {
    uint32_t WTO:16;        // 15:0 超时唤醒 rw 0x0000
    uint32_t :16;       // 31:16 保留位 - -
};

// 23.5.17 虚拟联网模式状态寄存器(CAN_PN_STAT) [0xB08] 复位值:0x00000080 32位
// 配置约束：仅32位字访问；标志位写1清零
struct [[nodiscard]] R32_CAN_PN_STAT {
    uint32_t :7;        // 6:0 保留位 - -
    uint32_t MMCNTS:1;      // 7 帧匹配计数状态 r 0x01
    uint32_t MMCNT:8;       // 15:8 帧匹配计数 r 0x00
    uint32_t WMS:1;         // 16 匹配唤醒标志状态 rc_w1 0x00
    uint32_t WTOS:1;        // 17 超时唤醒标志状态 rc_w1 0x00
    uint32_t :14;       // 31:18 保留位 - -
};

// 23.5.18 虚拟联网模式期望标识符0寄存器(CAN_PN_EID0) [0xB0C] 复位值:0x00000000 32位
// 配置约束：所有位仅暂停模式可配；仅32位字访问
struct [[nodiscard]] R32_CAN_PN_EID0 {
    uint32_t EIDF_ELT:29;   // 28:0 期望ID/ID下限 rw 0x00000000
    uint32_t ERTR:1;        // 29 期望RTR rw 0
    uint32_t EIDE:1;        // 30 期望IDE rw 0
    uint32_t :1;        // 31 保留位 - -
};

// 23.5.19 虚拟联网模式期望DLC寄存器(CAN_PN_EDLC) [0xB10] 复位值:0x00000008 32位
// 配置约束：所有位仅暂停模式可配；仅32位字访问
struct [[nodiscard]] R32_CAN_PN_EDLC {
    uint32_t DLCEHT:4;      // 3:0 期望DLC上限 rw 0x08
    uint32_t :12;       // 15:4 保留位 - -
    uint32_t DLCELT:4;      // 19:16 期望DLC下限 rw 0x00
    uint32_t :12;       // 31:20 保留位 - -
};

// 23.5.20 虚拟联网模式期望数据低字0寄存器(CAN_PN_EDL0) [0xB14] 复位值:0x00000000 32位
// 配置约束：所有位仅暂停模式可配；仅32位字访问
struct [[nodiscard]] R32_CAN_PN_EDL0 {
    uint32_t DB3ELT:8;      // 7:0 数据字节3下限/期望 rw 0x00
    uint32_t DB2ELT:8;      // 15:8 数据字节2下限/期望 rw 0x00
    uint32_t DB1ELT:8;      // 23:16 数据字节1下限/期望 rw 0x00
    uint32_t DB0ELT:8;      // 31:24 数据字节0下限/期望 rw 0x00
};

// 23.5.21 虚拟联网模式期望数据低字1寄存器(CAN_PN_EDL1) [0xB18] 复位值:0x00000000 32位
// 配置约束：所有位仅暂停模式可配；仅32位字访问
struct [[nodiscard]] R32_CAN_PN_EDL1 {
    uint32_t DB7ELT:8;      // 7:0 数据字节7下限/期望 rw 0x00
    uint32_t DB6ELT:8;      // 15:8 数据字节6下限/期望 rw 0x00
    uint32_t DB5ELT:8;      // 23:16 数据字节5下限/期望 rw 0x00
    uint32_t DB4ELT:8;      // 31:24 数据字节4下限/期望 rw 0x00
};

// 23.5.22 虚拟联网模式标识符过滤器/期望标识符1寄存器(CAN_PN_IFEID1) [0xB1C] 复位值:0x00000000 32位
// 配置约束：所有位仅暂停模式可配；仅32位字访问
struct [[nodiscard]] R32_CAN_PN_IFEID1 {
    uint32_t IDFD_EHT:29;   // 28:0 ID过滤数据/ID上限 rw 0x00000000
    uint32_t RTRFD:1;       // 29 RTR过滤数据 rw 0
    uint32_t IDEFD:1;       // 30 IDE过滤数据 rw 0
    uint32_t :1;        // 31 保留位 - -
};

// 23.5.23 虚拟联网模式数据0过滤器/期望数据高字0寄存器(CAN_PN_DF0EDH0) [0xB20] 复位值:0x00000000 32位
// 配置约束：所有位仅暂停模式可配；仅32位字访问
struct [[nodiscard]] R32_CAN_PN_DF0EDH0 {
    uint32_t DB3FD_EHT:8;   // 7:0 数据字节3过滤/上限 rw 0x00
    uint32_t DB2FD_EHT:8;   // 15:8 数据字节2过滤/上限 rw 0x00
    uint32_t DB1FD_EHT:8;   // 23:16 数据字节1过滤/上限 rw 0x00
    uint32_t DB0FD_EHT:8;   // 31:24 数据字节0过滤/上限 rw 0x00
};

// 23.5.24 虚拟联网模式数据1过滤器/期望数据高字1寄存器(CAN_PN_DF1EDH1) [0xB24] 复位值:0x00000000 32位
// 配置约束：所有位仅暂停模式可配；仅32位字访问
struct [[nodiscard]] R32_CAN_PN_DF1EDH1 {
    uint32_t DB7FD_HTF:8;   // 7:0 数据字节7过滤/上限 rw 0x00
    uint32_t DB6FD_HTF:8;   // 15:8 数据字节6过滤/上限 rw 0x00
    uint32_t DB5FD_HTF:8;   // 23:16 数据字节5过滤/上限 rw 0x00
    uint32_t DB4FD_HTF:8;   // 31:24 数据字节4过滤/上限 rw 0x00
};

// 23.5.25 虚拟联网模式接收唤醒邮箱x控制状态信息寄存器(CAN_PN_RWMxCS) [0xB40+16*x] 复位值:0x00000000 32位
// 配置约束：仅32位字访问
struct [[nodiscard]] R32_CAN_PN_RWMxCS {
    uint32_t :16;       // 15:0 保留位 - -
    uint32_t RDLC:4;        // 19:16 接收到的DLC域 r 0
    uint32_t RRTR:1;        // 20 接收到的RTR位 r 0
    uint32_t RIDE:1;        // 21 接收到的IDE位 r 0
    uint32_t RSRR:1;        // 22 接收到的SRR位 r 0
    uint32_t :9;        // 31:23 保留位 - -
};

// 23.5.26 虚拟联网模式接收唤醒邮箱x标识符寄存器(CAN_PN_RWMxI) [0xB44+16*x] 复位值:0x00000000 32位
// 配置约束：仅32位字访问
struct [[nodiscard]] R32_CAN_PN_RWMxI {
    uint32_t RID:29;        // 28:0 接收到的ID域 r 0x00000000
    uint32_t :3;        // 31:29 保留位 - -
};

// 23.5.27 虚拟联网模式接收唤醒邮箱x数据0寄存器(CAN_PN_RWMxD0) [0xB48+16*x] 复位值:0x00000000 32位
// 配置约束：仅32位字访问
struct [[nodiscard]] R32_CAN_PN_RWMxD0 {
    uint32_t RDB3:8;        // 7:0 接收到的数据字节3 r 0x00
    uint32_t RDB2:8;        // 15:8 接收到的数据字节2 r 0x00
    uint32_t RDB1:8;        // 23:16 接收到的数据字节1 r 0x00
    uint32_t RDB0:8;        // 31:24 接收到的数据字节0 r 0x00
};

// 23.5.28 虚拟联网模式接收唤醒邮箱x数据1寄存器(CAN_PN_RWMxD1) [0xB4C+16*x] 复位值:0x00000000 32位
// 配置约束：仅32位字访问
struct [[nodiscard]] R32_CAN_PN_RWMxD1 {
    uint32_t RDB7:8;        // 7:0 接收到的数据字节7 r 0x00
    uint32_t RDB6:8;        // 15:8 接收到的数据字节6 r 0x00
    uint32_t RDB5:8;        // 23:16 接收到的数据字节5 r 0x00
    uint32_t RDB4:8;        // 31:24 接收到的数据字节4 r 0x00
};

// 23.5.29 FD控制寄存器(CAN_FDCTL) [0xC00] 复位值:0x80000101 32位
// 配置约束：位17:16/15/12:8仅暂停模式可配；不被SWRST复位；仅32位字访问
struct [[nodiscard]] R32_CAN_FDCTL {
    uint32_t TDCV:6;        // 5:0 传输延迟补偿值 rc_w1 0x01
    uint32_t :2;        // 7:6 保留位 - -
    uint32_t TDCO:5;        // 12:8 传输延迟补偿偏置 rw 0x01
    uint32_t :1;        // 13 保留位 - -
    uint32_t TDCS:1;        // 14 传输延迟补偿状态 r 0
    uint32_t TDCEN:1;       // 15 传输延迟补偿使能 rw 1（仅暂停模式可配）
    uint32_t MDSZ:2;        // 17:16 邮箱数据大小 rw 0x00（仅暂停模式可配）
    uint32_t :13;       // 30:18 保留位 - -
    uint32_t BRSEN:1;       // 31 数据阶段波特率切换使能 rw 1
};

// 23.5.30 FD位时间寄存器(CAN_FDBT) [0xC04] 复位值:0x00000000 32位
// 配置约束：所有位仅暂停模式可配；不被SWRST复位；仅32位字访问
struct [[nodiscard]] R32_CAN_FDBT {
    uint32_t DPBS2:3;        // 2:0 保留位 - -
    uint32_t Resv1:2;       // 5:3 数据位相位缓冲段2 rw 0x00
    uint32_t DPBS1:3;       // 10:8 数据位相位缓冲段1 rw 0x00
    uint32_t :2;        // 7:6 保留位 - -
    uint32_t DPTS:5;        // 15:11 数据位传播时间段 rw 0x00
    uint32_t :1;        // 16 保留位 - -
    uint32_t DSJW:3;        // 19:17 数据位再同步补偿宽度 rw 0x00
    uint32_t :1;        // 20 保留位 - -
    uint32_t DBAUDPSC:10;   // 30:21 数据位波特率分频系数 rw 0x0000
    uint32_t :2;        // 31:30 保留位 - -
};

// 23.5.31 常规帧和FD帧CRC寄存器(CAN_CRCCFD) [0xC08] 复位值:0x00000000 32位
// 配置约束：仅32位字访问
struct [[nodiscard]] R32_CAN_CRCCFD {
    uint32_t CRCTCI:21;     // 20:0 常规/FD帧CRC计算值 r 0x0000000
    uint32_t :3;        // 23:21 保留位 - -
    uint32_t ANTM:5;        // 28:24 关联邮箱编号 r 0x00
    uint32_t :3;        // 31:29 保留位 - -
};

/*************************** 外设内存映射总结构体 ***************************/
struct [[nodiscard]] CANFD_Def {
    // 0x00 ~ 0x0C 核心控制寄存器段
    volatile R32_CAN_CTL0        CTL0;           // 0x00
    volatile R32_CAN_CTL1        CTL1;           // 0x04
    volatile R32_CAN_TIMER       TIMER;          // 0x08
    uint32_t                     __RESV1__;      // 0x0C 保留4字节

    // 0x10 ~ 0x1C 过滤+保留寄存器段
    volatile R32_CAN_RMPUBF      RMPUBF;         // 0x10
    uint32_t                     __RESV2__[1];   // 0x14 保留4字节
    volatile R32_CAN_ERR0        ERR0;           // 0x18
    uint32_t                     __RESV3__[1];   // 0x1C 保留4字节（ERR0偏移实际为0x18，后续偏移顺延）

    // 0x20 ~ 0x34 错误+中断+状态+控制2寄存器段
    volatile R32_CAN_ERR1        ERR1;           // 0x20
    uint32_t                     __RESV3a__[1];   // 
    volatile R32_CAN_INTEN       INTEN;          // 0x28
    uint32_t                     __RESV3b__[1];   // 
    volatile R32_CAN_STAT        STAT;           // 0x30
    volatile R32_CAN_CTL2        CTL2;           // 0x34

    // 0x38 ~ 0x4C CRC+FIFO过滤寄存器段
    uint32_t                     __RESV4__[3];   // 0x38-0x3C 保留8字节
    volatile R32_CAN_CRCC        CRCC;           // 0x44
    volatile R32_CAN_RFIFOPUBF   RFIFOPUBF;      // 0x48
    volatile R32_CAN_RFIFOIFMN   RFIFOIFMN;      // 0x4C

    // 0x50 ~ 0x87C 位时间+私有过滤寄存器段
    volatile R32_CAN_BT          BT;             // 0x50
    uint32_t                     __RESV5__[51 + ((2176 - 288) / 4)];
    volatile R32_CAN_RFIFOMPFx   RFIFOMPFx[32];  // 0x880-0x8FF 32个私有过滤寄存器（0x880+4*x）

    // 0x900 ~ 0xAFC 保留段（虚拟联网寄存器前预留）
    uint32_t                     __RESV6__[128]; // 0x900-0xAFF 保留512字节

    // 0xB00 ~ 0xB4B 虚拟联网模式寄存器段
    volatile R32_CAN_PN_CTL0     PN_CTL0;        // 0xB00
    volatile R32_CAN_PN_TO       PN_TO;          // 0xB04
    volatile R32_CAN_PN_STAT     PN_STAT;        // 0xB08
    volatile R32_CAN_PN_EID0     PN_EID0;        // 0xB0C
    volatile R32_CAN_PN_EDLC     PN_EDLC;        // 0xB10
    volatile R32_CAN_PN_EDL0     PN_EDL0;        // 0xB14
    volatile R32_CAN_PN_EDL1     PN_EDL1;        // 0xB18
    volatile R32_CAN_PN_IFEID1   PN_IFEID1;      // 0xB1C
    volatile R32_CAN_PN_DF0EDH0   PN_DF0EDH0;     // 0xB20
    volatile R32_CAN_PN_DF1EDH1   PN_DF1EDH1;     // 0xB24
    uint32_t                     __RESV7__[6];   // 0xB28-0xB3F 保留24字节
    // 虚拟联网接收唤醒邮箱（4个）
    volatile R32_CAN_PN_RWMxCS    PN_RWM0CS;      // 0xB40
    volatile R32_CAN_PN_RWMxI     PN_RWM0I;       // 0xB44
    volatile R32_CAN_PN_RWMxD0    PN_RWM0D0;      // 0xB48
    volatile R32_CAN_PN_RWMxD1    PN_RWM0D1;      // 0xB4C
    volatile R32_CAN_PN_RWMxCS    PN_RWM1CS;      // 0xB50
    volatile R32_CAN_PN_RWMxI     PN_RWM1I;       // 0xB54
    volatile R32_CAN_PN_RWMxD0    PN_RWM1D0;      // 0xB58
    volatile R32_CAN_PN_RWMxD1    PN_RWM1D1;      // 0xB5C
    volatile R32_CAN_PN_RWMxCS    PN_RWM2CS;      // 0xB60
    volatile R32_CAN_PN_RWMxI     PN_RWM2I;       // 0xB64
    volatile R32_CAN_PN_RWMxD0    PN_RWM2D0;      // 0xB68
    volatile R32_CAN_PN_RWMxD1    PN_RWM2D1;      // 0xB6C
    volatile R32_CAN_PN_RWMxCS    PN_RWM3CS;      // 0xB70
    volatile R32_CAN_PN_RWMxI     PN_RWM3I;       // 0xB74
    volatile R32_CAN_PN_RWMxD0    PN_RWM3D0;      // 0xB78
    volatile R32_CAN_PN_RWMxD1    PN_RWM3D1;      // 0xB7C

    // 0xB80 ~ 0xBFC 保留段（FD寄存器前预留）
    uint32_t                     __RESV8__[32];  // 0xB80-0xBFF 保留128字节

    // 0xC00 ~ 0xC08 FD专用寄存器段
    volatile R32_CAN_FDCTL       FDCTL;          // 0xC00
    volatile R32_CAN_FDBT        FDBT;           // 0xC04
    volatile R32_CAN_CRCCFD      CRCCFD;         // 0xC08

    // 寄存器大小静态断言（校验位域结构体实际字节数与硬件匹配）
    VALIDATE_R32(R32_CAN_CTL0)        ;
    VALIDATE_R32(R32_CAN_CTL1)        ;
    VALIDATE_R32(R32_CAN_TIMER)       ;
    VALIDATE_R32(R32_CAN_RMPUBF)      ;
    VALIDATE_R32(R32_CAN_ERR0)        ;
    VALIDATE_R32(R32_CAN_ERR1)        ;
    VALIDATE_R32(R32_CAN_INTEN)       ;
    VALIDATE_R32(R32_CAN_STAT)        ;
    VALIDATE_R32(R32_CAN_CTL2)        ;
    VALIDATE_R32(R32_CAN_CRCC)        ;
    VALIDATE_R32(R32_CAN_RFIFOPUBF)   ;
    VALIDATE_R32(R32_CAN_RFIFOIFMN)   ;
    VALIDATE_R32(R32_CAN_BT)          ;
    VALIDATE_R32(R32_CAN_RFIFOMPFx)   ;
    VALIDATE_R32(R32_CAN_PN_CTL0)     ;
    VALIDATE_R32(R32_CAN_PN_TO)       ;
    VALIDATE_R32(R32_CAN_PN_STAT)     ;
    VALIDATE_R32(R32_CAN_PN_EID0)     ;
    VALIDATE_R32(R32_CAN_PN_EDLC)     ;
    VALIDATE_R32(R32_CAN_PN_EDL0)     ;
    VALIDATE_R32(R32_CAN_PN_EDL1)     ;
    VALIDATE_R32(R32_CAN_PN_IFEID1)   ;
    VALIDATE_R32(R32_CAN_PN_DF0EDH0)   ;
    VALIDATE_R32(R32_CAN_PN_DF1EDH1)   ;
    VALIDATE_R32(R32_CAN_PN_RWMxCS)    ;
    VALIDATE_R32(R32_CAN_PN_RWMxI)     ;
    VALIDATE_R32(R32_CAN_PN_RWMxD0)    ;
    VALIDATE_R32(R32_CAN_PN_RWMxD1)    ;
    VALIDATE_R32(R32_CAN_FDCTL)       ;
    VALIDATE_R32(R32_CAN_FDBT)        ;
    VALIDATE_R32(R32_CAN_CRCCFD)      ;
};

/*************************** 偏移地址静态断言校验 ***************************/
// 基于CANFD_Def起始地址为0x00的硬件偏移校验，与手册地址完全匹配
static_assert(offsetof(CANFD_Def, CTL0)           == 0x00, "CAN_CTL0 offset error!");
static_assert(offsetof(CANFD_Def, CTL1)           == 0x04, "CAN_CTL1 offset error!");
static_assert(offsetof(CANFD_Def, TIMER)          == 0x08, "CAN_TIMER offset error!");
static_assert(offsetof(CANFD_Def, RMPUBF)         == 0x10, "CAN_RMPUBF offset error!");
static_assert(offsetof(CANFD_Def, ERR0)           == 0x18, "CAN_ERR0 offset error!");
static_assert(offsetof(CANFD_Def, ERR1)           == 0x20, "CAN_ERR1 offset error!");
static_assert(offsetof(CANFD_Def, INTEN)          == 0x28, "CAN_INTEN offset error!");
static_assert(offsetof(CANFD_Def, STAT)           == 0x30, "CAN_STAT offset error!");
static_assert(offsetof(CANFD_Def, CTL2)           == 0x34, "CAN_CTL2 offset error!");
static_assert(offsetof(CANFD_Def, CRCC)           == 0x44, "CAN_CRCC offset error!");
static_assert(offsetof(CANFD_Def, RFIFOPUBF)      == 0x48, "CAN_RFIFOPUBF offset error!");
static_assert(offsetof(CANFD_Def, RFIFOIFMN)      == 0x4C, "CAN_RFIFOIFMN offset error!");
static_assert(offsetof(CANFD_Def, BT)             == 0x50, "CAN_BT offset error!");
static_assert(offsetof(CANFD_Def, RFIFOMPFx[0])   == 0x880, "CAN_RFIFOMPFx[0] offset error!");
static_assert(offsetof(CANFD_Def, PN_CTL0)        == 0xB00, "CAN_PN_CTL0 offset error!");
static_assert(offsetof(CANFD_Def, PN_TO)          == 0xB04, "CAN_PN_TO offset error!");
static_assert(offsetof(CANFD_Def, PN_STAT)        == 0xB08, "CAN_PN_STAT offset error!");
static_assert(offsetof(CANFD_Def, PN_EID0)        == 0xB0C, "CAN_PN_EID0 offset error!");
static_assert(offsetof(CANFD_Def, PN_EDLC)        == 0xB10, "CAN_PN_EDLC offset error!");
static_assert(offsetof(CANFD_Def, PN_EDL0)        == 0xB14, "CAN_PN_EDL0 offset error!");
static_assert(offsetof(CANFD_Def, PN_EDL1)        == 0xB18, "CAN_PN_EDL1 offset error!");
static_assert(offsetof(CANFD_Def, PN_IFEID1)      == 0xB1C, "CAN_PN_IFEID1 offset error!");
static_assert(offsetof(CANFD_Def, PN_DF0EDH0)      == 0xB20, "CAN_PN_DF0EDH0 offset error!");
static_assert(offsetof(CANFD_Def, PN_DF1EDH1)      == 0xB24, "CAN_PN_DF1EDH1 offset error!");
static_assert(offsetof(CANFD_Def, PN_RWM0CS)       == 0xB40, "CAN_PN_RWM0CS offset error!");
static_assert(offsetof(CANFD_Def, FDCTL)          == 0xC00, "CAN_FDCTL offset error!");
static_assert(offsetof(CANFD_Def, FDBT)           == 0xC04, "CAN_FDBT offset error!");
static_assert(offsetof(CANFD_Def, CRCCFD)         == 0xC08, "CAN_CRCCFD offset error!");

}
