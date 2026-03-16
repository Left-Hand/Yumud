#pragma once

#include "core/constants/enums.hpp"
#include "core/io/regs.hpp"

//使用ptb/stb模型

namespace ymd::ral::can::py32f403_specified{

// DLC 数据长度代码
// DLC 定义了帧内的有效载荷字节数（表 3-42）。
// 经典 CAN 2.0B 帧的 DLC 长度为 4 位 (DLC(3:0))。
// 对于经典 CAN 2.0B，可以选择传输 DLC 没有意义的远程帧。位 RMF 的解释提供了更多细节。
// IDE 标识符扩展位
// 0 – 标准格式: ID(28:18)
// 1 – 扩展格式: ID(28:0)
// FDF CAN FD 帧标志位
// 该位应当设置为0从而支持 CAN 2.0 frame
// BRS CAN FD 比特率开关使能
// 0 – 完整帧的标称/慢比特率
// 1 – 为数据有效负载和 CAN FD 帧的 CRC 切换到数据/快速比特率
// BRS 对经典 CAN 没有意义。
// RMF 远程帧标志位
// 0 – 数据帧
// 1 – 远程帧
// 远程帧携带零字节有效载荷。DLC 的值按原样传输，但对帧大小没有影响。
// 因此，远程帧的 DLC 值可能携带一些编码信息。
// 只有经典的 CAN 2.0B 帧可以是远程帧。
// KOER 错误类型
// 接收帧的 KOER 与寄存器 EALCAP 中的 KOER 位具有相同的含义。
// 如果 RBALL=1（第 4.10.4 章），则接收帧的 KOER 变得有意义。
// 请注意，如果 RBALL=1，一般情况下会禁用过滤器组。
// ESI 错误状态指示器
// 协议机器自动将 ESI 的正确值嵌入到传输的帧中。
// 0 – CAN 节点处于错误主动状态
// 1 – CAN 节点错误被动状态
// ESI 仅包含在 CAN FD 帧中，因此对于接收的经典 CAN 2.0B 始终为 0。
// 传输的错误状态由寄存器 INTF 中的位 EPASS 显示。
// LBF 环回帧标志位
// 如果激活了环回模式并且 CAN 控制器已接收到自己的传输帧，则接收帧的 LBF 设置为 1。
// 如果 LBME=1 而网络中的其他节点也进行传输，这将很有用。
struct [[nodiscard]] R32_CAN_FORMAT
{
    uint32_t DLC:11;        // 数据长度代码 11位
    uint32_t RES0:5;        // 保留位 5位

    uint32_t IDE:1;         // 标识符扩展位 1位
    uint32_t FDF:1;         // CAN FD帧标志位 1位
    uint32_t BRS:1;         // 比特率开关使能 1位
    uint32_t XLF:1;         // 保留/扩展帧标志 1位
    uint32_t RMF:1;         // 远程帧标志位 1位
    uint32_t SEC:1;         // 保留/安全帧标志 1位
    uint32_t RES1:2;        // 保留位 2位
    uint32_t KOER:3;        // 错误类型 3位
    uint32_t ESI:1;         // 错误状态指示器 1位
    uint32_t LBF:1;         // 环回帧标志位 1位
    uint32_t RES2:3;        // 保留位 3位
};

// HANDLE 帧识别句柄
// 句柄的目的是使用 TSTAT 识别帧。MAC 帧中不使用 HANDLE。
// 建议主机应用程序将软件计数器的值写入 HANDLE。
// 这样的软件计数器可以随着要传输的每个新帧而增加，并且应该翻转。
// 注意：为避免在模拟过程中出现未初始化的内存问题，
// 主机应用程序应始终为要传输的每一帧定义 HANDLE。
struct [[nodiscard]] R32_CAN_TYPE
{
    uint32_t RES0:24;       // 保留位 24位
    uint32_t HANDLE:8;      // 帧识别句柄 8位
};

// CYCLE_TIME 周期时间（TTCAN 的时间戳）
// CYCLE_TIME 将仅存储接收到的帧。这是帧 SOF 处的循环时间。
// 参考消息的循环时间始终为 0。
// AF 接受域
// 建议主要用于帧接受筛选，保留位全写 0xFFFFFFFF。
struct [[nodiscard]] R32_CAN_AF
{
    uint32_t BITS;          // 接受域筛选位 32位
};

struct [[nodiscard]] R32_CAN_TTCAN
{
    uint32_t CYCLE_TIME:16; // TTCAN周期时间/时间戳 16位
    uint32_t RES0:16;       // 保留位 16位
};

// Payload Data 帧的有效载荷数据
// 经典 CAN 2.0B 最多 8 个字节，CANFD 最多 64个字节。
// 标志id和拓展id都从最低位开始
struct CAN_LLC_FRAME
{
    volatile uint32_t ID;               // 帧ID 32位 [0x00]
    volatile R32_CAN_FORMAT FORMAT;     // 帧格式寄存器 [0x04]
    volatile R32_CAN_TYPE TYPE;         // 帧类型寄存器 [0x08]
    volatile R32_CAN_AF AF;             // 帧接受域寄存器 [0x0C]
    uint32_t __RESV1__[2];              // 保留空间 8字节 [0x10-0x17]
    volatile R32_CAN_TTCAN TTCAN;       // TTCAN时间戳寄存器 [0x18]
    volatile uint32_t DATA[16];         // 帧有效载荷数据 64字节 [0x1C-0x5B]
};

static_assert(__builtin_offsetof(CAN_LLC_FRAME, ID)      == 0x00, "CAN_LLC_FRAME ID offset error!");
static_assert(__builtin_offsetof(CAN_LLC_FRAME, FORMAT)  == 0x04, "CAN_LLC_FRAME FORMAT offset error!");
static_assert(__builtin_offsetof(CAN_LLC_FRAME, TYPE)    == 0x08, "CAN_LLC_FRAME TYPE offset error!");
static_assert(__builtin_offsetof(CAN_LLC_FRAME, AF)      == 0x0C, "CAN_LLC_FRAME AF offset error!");
static_assert(__builtin_offsetof(CAN_LLC_FRAME, TTCAN)  == 0x18, "CAN_LLC_FRAME TTCAN offset error!");
static_assert(__builtin_offsetof(CAN_LLC_FRAME, DATA)    == 0x1C, "CAN_LLC_FRAME DATA offset error!");
static_assert(sizeof(CAN_LLC_FRAME)    == 0xCC - 0x70, "CAN_LLC_FRAME size error!");


// 24.4.1 节点配置寄存器(CANFD_TSNCR) [0x00] 复位值:0x02010801 32位
struct [[nodiscard]] R32_CANFD_TSNCR {
    uint32_t VERSION:16;    // 15:0 CAN-CTRL版本 r 0x0801
    uint32_t CES:1;         // 16 CAN错误信号 rw 1
    uint32_t ROP:1;         // 17 限制操作 rw 0
    uint32_t Res1:14;       // 31:18 保留位 - -
};

// 24.4.2 位时序配置寄存器(CANFD_ACBTR) [0x04] 复位值:0x05050008 32位
struct [[nodiscard]] R32_CANFD_ACBTR {
    uint32_t AC_SEG_1:9;    // 8:0 位时序段1 rw 0x08
    uint32_t Res1:7;        // 15:9 保留位 - -
    uint32_t AC_SEG_2:7;    // 22:16 位时序段2 rw 0x05
    uint32_t Res2:1;        // 23 保留位 - -
    uint32_t AC_SJW:7;      // 30:24 同步跳转宽度 rw 0x05
    uint32_t Res3:1;        // 31 保留位 - -
};

// 24.4.3 CANFD_FDBTR [0x08] 复位值:0x02020003 32位
struct [[nodiscard]] R32_CANFD_FDBTR {
    uint32_t FD_SEG_1:8;    // 7:0 CAN FD位时序段1 rw 0x03
    uint32_t Res1:8;        // 15:8 保留位 - -
    uint32_t FD_SEG_2:7;    // 22:16 CAN FD位时序段2 rw 0x02
    uint32_t Res2:1;        // 23 保留位 - -
    uint32_t FD_SJW:7;      // 30:24 CAN FD同步跳转宽度 rw 0x02
    uint32_t Res3:1;        // 31 保留位 - -
};

// 24.4.4 限制与预分频配置寄存器(CANFD_RLSSP) [0x10] 复位值:0x77000000 32位
struct [[nodiscard]] R32_CANFD_RLSSP {
    uint32_t PRESC:5;       // 4:0 预分频器 rw 0x00
    uint32_t Res1:3;        // 7:5 保留位 - -
    uint32_t FD_SSPOFF:8;   // 15:8 二次采样点偏移 rw 0x00
    uint32_t Res2:8;        // 23:16 保留位 - -
    uint32_t REALIM:3;      // 26:24 重新仲裁限制 rw 0x07
    uint32_t Res3:1;        // 27 保留位 - -
    uint32_t RETLIM:3;      // 30:28 重传尝试限制 rw 0x07
    uint32_t Res4:1;        // 31 保留位 - -
};

// 24.4.5 状态寄存器(CANFD_IFR) [0x14] 复位值:0x00000000 32位
struct [[nodiscard]] R32_CANFD_IFR {
    uint32_t AIF:1;         // 0 取消发送中断标志 rw 0
    uint32_t EIF:1;         // 1 错误中断标志 rw 0
    uint32_t Res1:1;        // 2 保留位 - -
    uint32_t RIF:1;         // 3 接收中断标志 rw 0
    uint32_t RAFIF:1;       // 4 接收BUF将满中断标志 rw 0
    uint32_t RFIF:1;        // 5 接收BUF满中断标志 rw 0
    uint32_t ROIF:1;        // 6 接收上溢中断标志 rw 0
    uint32_t BEIF:1;        // 7 总线错误中断标志 rw 0
    uint32_t ALIF:1;        // 8 仲裁失败中断标志 rw 0
    uint32_t TPIF:1;        // 9 PTB发送中断标志 rw 0
    uint32_t EPIF:1;        // 10 错误被动中断标志 rw 0
    uint32_t TTIF:1;        // 11 时间触发中断标志 rw 0
    uint32_t TEIF:1;        // 12 触发错误中断标志 rw 0
    uint32_t WTIF:1;        // 13 触发看门中断标志 rw 0
    uint32_t Res2:2;        // 15:14 保留位 - -
    uint32_t Res3:14;       // 29:16 保留位 - -
    uint32_t EPASS:1;       // 30 错误被动 r 0
    uint32_t EWARN:1;       // 31 错误警告阈值 r 0
};

// 24.4.6 中断使能寄存器(CANFD_IER) [0x18] 复位值:0x000468FE 32位
struct [[nodiscard]] R32_CANFD_IER {
    uint32_t Res1:1;        // 0 保留位 - -
    uint32_t EIE:1;         // 1 错误中断使能 rw 1
    uint32_t TSIE:1;        // 2 STB发送中断使能 rw 1
    uint32_t TPIE:1;        // 3 PTB发送中断使能 rw 1
    uint32_t RAFIE:1;       // 4 接收BUF将满中断使能 rw 1
    uint32_t RFIE:1;        // 5 接收BUF满中断使能 rw 1
    uint32_t ROIE:1;        // 6 接收上溢中断使能 rw 1
    uint32_t RIE:1;         // 7 接收中断使能 rw 1
    uint32_t BEIE:1;        // 8 总线错误中断使能 rw 1
    uint32_t ALIE:1;        // 9 仲裁失败中断使能 rw 1
    uint32_t EPIE:1;        // 10 错误被动中断使能 rw 1
    uint32_t TTIE:1;        // 11 时间触发中断使能 rw 1
    uint32_t Res2:1;        // 12 保留位 - -
    uint32_t WTIE:1;        // 13 触发看门中断使能 rw 1
    uint32_t Res3:2;        // 15:14 保留位 - -
    uint32_t Res4:16;       // 31:16 保留位 - -
};

// 24.4.7 传输状态寄存器(CANFD_TSR) [0x1C] 复位值:0x00000000 32位
struct [[nodiscard]] R32_CANFD_TSR {
    uint32_t HANDLE_L:8;    // 7:0 帧识别handle低8位 rw 0
    uint32_t TSTAT_L:3;     // 10:8 传输状态码低3位 rw 0
    uint32_t Res1:5;        // 15:11 保留位 - -
    uint32_t HANDLE_H:8;    // 23:16 帧识别handle高8位 rw 0
    uint32_t TSTAT_H:3;     // 26:24 传输状态码高3位 rw 0
    uint32_t Res2:5;        // 31:27 保留位 - -
};

// 24.4.8 全局配置寄存器(CANFD_MCR) [0x28] 复位值:0x00900080 32位
struct [[nodiscard]] R32_CANFD_MCR {
    uint32_t BUSOFF:1;      // 0 总线关闭 rw 0
    uint32_t Res1:4;        // 4:1 保留位 - -
    uint32_t LBMI:1;        // 5 内部回环模式使能 rw 0
    uint32_t LBME:1;        // 6 外部回环模式使能 rw 0
    uint32_t RESET:1;       // 7 复位请求 rw 1
    uint32_t TSA:1;         // 8 STB发送取消 rw 0
    uint32_t TSALL:1;       // 9 发送所有STB数据 rw 0
    uint32_t TSONE:1;       // 10 发送一帧STB数据 rw 0
    uint32_t TPA:1;         // 11 PTB发送取消 rw 0
    uint32_t TPE:1;         // 12 PTB发送使能 rw 0
    uint32_t STBY:1;        // 13 收发器待机 rw 0
    uint32_t LOM:1;         // 14 静默模式使能 rw 0
    uint32_t TBSEL:1;       // 15 发送BUF选择 rw 0
    uint32_t TSSTAT:2;      // 17:16 STB状态 r 0
    uint32_t TSFF:1;        // 18 发送BUF满标志 r 0
    uint32_t Res2:1;        // 19 保留位 - -
    uint32_t TTTBM:1;       // 20 TTCAN BUF模式 rw 1
    uint32_t TSMODE:1;      // 21 STB发送模式 rw 0
    uint32_t TSNEXT:1;      // 22 下一个STB SLOT rw 0
    uint32_t FD_ISO:1;      // 23 CAN FD ISO模式 rw 1
    uint32_t RSTAT:2;       // 25:24 接收缓冲区状态 r 0
    uint32_t Res3:1;        // 26 保留位 - -
    uint32_t RBALL:1;       // 27 接收BUF存储所有帧 rw 0
    uint32_t RREL:1;        // 28 释放接收BUF rw 0
    uint32_t ROV:1;         // 29 接收BUF上溢标志 r 0
    uint32_t ROM:1;         // 30 接收BUF上溢模式 rw 0
    uint32_t SACK:1;        // 31 自应答 rw 0
};

// 24.4.9 错误警告寄存器(CANFD_WECR) [0x2C] 复位值:0x0000001B 32位
struct [[nodiscard]] R32_CANFD_WECR {
    uint32_t EWL:4;         // 3:0 错误警告阈值 rw 0x0B
    uint32_t AFWL:4;        // 7:4 接收BUF将满警告值 rw 0x01
    uint32_t ALC:5;         // 12:8 仲裁失败位置捕捉 r 0
    uint32_t KOER:3;        // 15:13 错误类别 rw 0
    uint32_t RECNT:8;       // 23:16 接收错误计数器 r 0
    uint32_t TECNT:8;       // 31:24 发送错误计数器 r 0
};

// 24.4.10 参考ID寄存器(CANFD_REFMSG) [0x30] 复位值:0x00000000 32位
struct [[nodiscard]] R32_CANFD_REFMSG {
    uint32_t REF_ID_L:16;   // 15:0 参考消息ID低16位 rw 0
    uint32_t REF_ID_H:13;   // 28:16 参考消息ID高13位 rw 0
    uint32_t Res1:2;        // 30:29 保留位 - -
    uint32_t REF_IDE:1;     // 31 参考消息IDE位 rw 0
};

// 24.4.11 TTCAN配置寄存器(CANFD_TTCR) [0x34] 复位值:0x00000000 32位
struct [[nodiscard]] R32_CANFD_TTCR {
    uint32_t TTPTR:6;       // 5:0 发送触发器TB slot指针 rw 0
    uint32_t TTYPE:3;       // 8:6 触发类型 rw 0
    uint32_t Res1:1;        // 9 保留位 - -
    uint32_t TEW:4;         // 13:10 发送使能窗口 rw 0
    uint32_t Res2:3;        // 16:14 保留位 - -
    uint32_t TBPTR:6;       // 22:17 TB SLOT指针 rw 0
    uint32_t TBF:1;         // 23 设置TB为已填充 rw 0
    uint32_t TBE:1;         // 24 设置TB为空 rw 0
    uint32_t Res3:2;        // 26:25 保留位 - -
    uint32_t T_PRESC:2;     // 28:27 TTCAN定时器预分频 rw 0
    uint32_t TTEN:1;        // 29 TTCAN使能 rw 0
    uint32_t Res4:2;        // 31:30 保留位 - -
};

// 24.4.12 TTCAN触发寄存器(CANFD_TTTR) [0xFFFF0000] 复位值:0x02020003 32位
struct [[nodiscard]] R32_CANFD_TTTR {
    uint32_t TT_TRIG:16;    // 15:0 触发时间 rw 0x0003
    uint32_t TT_WTRIG:16;   // 31:16 看门触发时间 rw 0x0202
};

// 24.4.13 CANFD_SCMS [0x3C] 复位值:0x00000000 32位
struct [[nodiscard]] R32_CANFD_SCMS {
    uint32_t Res1:1;        // 0 保留位 - -
    uint32_t FSTIM:3;       // 3:1 故障刺激 rw 0
    uint32_t Res2:20;       // 23:4 保留位 - -
    uint32_t ACFA:1;        // 24 接受过滤器接收 rw 0
    uint32_t TXS:1;         // 25 传输停止 r 0
    uint32_t TXB:1;         // 26 传输受阻 r 0
    uint32_t HELOC:2;       // 28:27 主机端内存错误位置 r 0
    uint32_t Res3:3;        // 31:29 保留位 - -
};

// 24.4.14 筛选器组控制寄存器(CANFD_ACFCR) [0x44] 复位值:0x00010000 32位
struct [[nodiscard]] R32_CANFD_ACFCR {
    uint32_t ACFADR:4;      // 3:0 筛选器地址 rw 0
    uint32_t Res1:12;       // 15:4 保留位 - -
    uint32_t AE_0:1;        // 16 ACF0使能 rw 1
    uint32_t AE_11:1;       // 17 ACF11使能 rw 0
    uint32_t AE_10:1;       // 18 ACF10使能 rw 0
    uint32_t AE_4:1;        // 19 ACF4使能 rw 0
    uint32_t AE_2:1;        // 20 ACF2使能 rw 0
    uint32_t AE_3:1;        // 21 ACF3使能 rw 0
    uint32_t AE_5:1;        // 22 ACF5使能 rw 0
    uint32_t AE_6:1;        // 23 ACF6使能 rw 0
    uint32_t AE_9:1;        // 24 ACF9使能 rw 0
    uint32_t AE_8:1;        // 25 ACF8使能 rw 0
    uint32_t AE_1:1;        // 26 ACF1使能 rw 0
    uint32_t AE_7:1;        // 27 ACF7使能 rw 0
    uint32_t Res2:4;        // 31:28 保留位 - -
};

// 24.4.15 筛选器组code寄存器(CANFD_ACFC) [0x48] 复位值:0xXXXXXXXX 32位
struct [[nodiscard]] R32_CANFD_ACFC {
    uint32_t VAL:32;        // 31:0 寄存器值(分ID/FORMAT/TYPE段) rw 0xXXXXXXXX
};

// 24.4.16 筛选器组mask寄存器(CANFD_ACFM) [0x58] 复位值:0xXXXXXXXX 32位
struct [[nodiscard]] R32_CANFD_ACFM {
    uint32_t VAL:32;        // 31:0 寄存器值(分ID/FORMAT/TYPE段) rw 0xXXXXXXXX
};

// 24.4.17 CAN接收BUF寄存器(CANFD_RBUF) [0x70] 复位值:0xXXXXXXXX 32位
struct [[nodiscard]] R32_CANFD_RBUF {
    uint32_t VAL:32;        // 31:0 接收BUF数据 rw 0xXXXXXXXX
};

// 24.4.18 CAN发送BUF寄存器(CANFD_TBUF) [0xCC] 复位值:0xXXXXXXXX 32位
struct [[nodiscard]] R32_CANFD_TBUF {
    uint32_t VAL:32;        // 31:0 发送BUF数据 rw 0xXXXXXXXX
};

/*************************** 外设内存映射总结构体 ***************************/
struct [[nodiscard]] CANFD_Def {
    volatile R32_CANFD_TSNCR    TSNCR;      // 0x00
    volatile R32_CANFD_ACBTR    ACBTR;      // 0x04
    volatile R32_CANFD_FDBTR    FDBTR;      // 0x08
    uint32_t                    __RESV1__;  // 0x0C 保留4字节

    volatile R32_CANFD_RLSSP    RLSSP;      // 0x10
    volatile R32_CANFD_IFR      IFR;        // 0x14
    volatile R32_CANFD_IER      IER;        // 0x18
    volatile R32_CANFD_TSR      TSR;        // 0x1C

    uint32_t                    __RESV2__[2];// 0x20-0x24 保留8字节
    volatile R32_CANFD_MCR      MCR;        // 0x28
    volatile R32_CANFD_WECR     WECR;       // 0x2C

    volatile R32_CANFD_REFMSG   REFMSG;     // 0x30
    volatile R32_CANFD_TTCR     TTCR;       // 0x34
    uint32_t                     __RESV3__;
    volatile R32_CANFD_SCMS     SCMS;       // 0x3C

    uint32_t                    __RESV4__;  // 0x40 保留4字节
    volatile R32_CANFD_ACFCR    ACFCR;      // 0x44
    volatile R32_CANFD_ACFC     ACFC;       // 0x48

    uint32_t                    __RESV5__[3];// 0x4C-0x54 保留12字节
    volatile R32_CANFD_ACFM     ACFM;       // 0x58

    uint32_t                    __RESV6__[5];// 0x5C-0x6C 保留20字节
    volatile CAN_LLC_FRAME     RBUF;       // 0x70

    volatile CAN_LLC_FRAME     TBUF;       // 0xCC

    VALIDATE_R32(R32_CANFD_TSNCR)  ;
    VALIDATE_R32(R32_CANFD_ACBTR)  ;
    VALIDATE_R32(R32_CANFD_FDBTR)  ;
    VALIDATE_R32(R32_CANFD_RLSSP)  ;
    VALIDATE_R32(R32_CANFD_IFR)    ;
    VALIDATE_R32(R32_CANFD_IER)    ;
    VALIDATE_R32(R32_CANFD_TSR)    ;
    VALIDATE_R32(R32_CANFD_MCR)    ;
    VALIDATE_R32(R32_CANFD_WECR)   ;
    VALIDATE_R32(R32_CANFD_REFMSG) ;
    VALIDATE_R32(R32_CANFD_TTCR)   ;
    VALIDATE_R32(R32_CANFD_TTTR)   ;
    VALIDATE_R32(R32_CANFD_SCMS)   ;
    VALIDATE_R32(R32_CANFD_ACFCR)  ;
    VALIDATE_R32(R32_CANFD_ACFC)   ;
    VALIDATE_R32(R32_CANFD_ACFM)   ;
    VALIDATE_R32(R32_CANFD_RBUF)   ;
    VALIDATE_R32(R32_CANFD_TBUF)   ;
};


static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::TSNCR)   == 0x00, "TSNCR offset error!");
static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::ACBTR)   == 0x04, "ACBTR offset error!");
static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::FDBTR)   == 0x08, "FDBTR offset error!");
static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::RLSSP)   == 0x10, "RLSSP offset error!");
static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::IFR)     == 0x14, "IFR offset error!");
static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::IER)     == 0x18, "IER offset error!");
static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::TSR)     == 0x1C, "TSR offset error!");
static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::MCR)     == 0x28, "MCR offset error!");
static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::WECR)    == 0x2C, "WECR offset error!");
static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::REFMSG)  == 0x30, "REFMSG offset error!");
static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::TTCR)    == 0x34, "TTCR offset error!");
static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::SCMS)    == 0x3C, "SCMS offset error!");
static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::ACFCR)   == 0x44, "ACFCR offset error!");
static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::ACFC)    == 0x48, "ACFC offset error!");
static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::ACFM)    == 0x58, "ACFM offset error!");
static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::RBUF)    == 0x70, "RBUF offset error!");
static_assert(__builtin_offsetof(CANFD_Def, CANFD_Def::TBUF)    == 0xCC, "TBUF offset error!");


}