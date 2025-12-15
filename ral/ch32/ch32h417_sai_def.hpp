#pragma once

#include <cstdint>
#include "core/io/regs.hpp"


#ifndef ASSERT_REG_IS_32BIT
#define ASSERT_REG_IS_32BIT(name) static_assert((sizeof(name) == 4),  #name " is not 4 bytes"); 
#endif

namespace ymd::ral::CH32{

struct [[nodiscard]] R32_SAI_xCFGR1{
    // SAIx 音频模块模式：
    // 00：主发送器；
    // 01：主接收器；
    // 10：从发送器；
    // 11：从接收器。
    // 注：（1）如果将音频模块配置为 SPDIF 模式，则
    // 将强制设置主发送模式（MODE[1:0] = 00）。在主
    // 发送模式下，音频模块将立即开始生成 FS 和时钟；
    // （2）仅在音频模块禁止情况下配置该位。
    uint32_t MODE:2;
    uint32_t PRTCFG:2;
    uint32_t :1;

    // 数据大小选择：
    // 000/001：保留；
    // 010：8 位；
    // 011：10 位；
    // 100：16 位；
    // 101：20 位；
    // 110：24 位；
    // 111：32 位。
    // 注：仅在音频模块禁止情况下配置该位。
    uint32_t DS:3;
    uint32_t LSBFIRST:1;
    uint32_t :1;

    // 时钟选通边沿：
    // 1：在 SCK 下降沿更改 SAI 生成的信号，而在 SCK 
    // 上升沿对 SAI 接收的信号进行采样；
    // 0：在 SCK 上升沿更改 SAI 生成的信号，而在 SCK 
    // 下降沿对 SAI 接收的信号进行采样。
    // 注：仅在音频模块禁止情况下配置该位。
    uint32_t CKSTR:1;
    uint32_t SYNCEN:2;
    uint32_t MONO:1;
    uint32_t OUTDRIV:1;
    uint32_t :2;
    uint32_t SAIEN:1;
    uint32_t DMAEN:1;
    uint32_t :1;
    uint32_t NODIV:1;
    uint32_t MCKDIV:6;
    uint32_t OSR:1;
    uint32_t :5;
};

struct [[nodiscard]] R32_SAI_xCFGR2{
    // 000：FIFO 为空；
    // 001：1/4
    // 010：1/2
    // 011：3/4；
    // 100：FIFO 已满；
    uint32_t FTH:3;

    // FIFO 刷新：
    // 1：FIFO 刷新。将此位编程为 1 可触发 FIFO 刷新。
    // 所有的内部 FIFO 指针（读和写）将清零。这种情
    // 况下，仍存留在 FIFO 中的数据丢失（发送或接收
    // 数据不会继续丢失）。刷新 SAI 前，必须禁止 DMA
    // 数据流/中断；
    // 0：禁止 FIFO 刷新。
    uint32_t FFLUSH:1;

    // 数据线的三态管理：
    // 1：SD 输出线将在上一个有效 Slot（下一个 Slot 
    // 无效）的最后一个数据位传输结束时释放（高阻
    // 态）；
    // 0：Slot 无效时，SD 输出线仍由 SAI 驱动。
    uint32_t TRIS:1;

    // 静音模式使能：
    // 1：使能静音模式；
    // 0：禁止静音模式。
    // 注：该位对 SPDIF 音频模块无意义，从而也不使
    // 用。
    uint32_t MUTE:1;
    uint32_t MUTEVAL:1;

    // 静音计数器：
    // 这些位中所设置的值将与接收模式下检测到的连
    // 续静音帧数量进行比较。当静音帧数量与该值相
    // 等时，MUTEDET标志置1，并且在MUTEDETIE位置1
    // 的情况下，还将生成中断。
    // 注：该域仅用于接收模式。
    uint32_t MUTE_CNT:6;

    // 补码位：
    // 1：2 的补码表示；
    // 0：1 的补码表示。
    // 注：仅当压扩模式为 μ-Law 算法或 A-Law 算法时
    // 该位才有效。
    uint32_t CPL:1;

    // 压扩模式：
    // 00：不支持压扩算法；
    // 01：保留；
    // 10：μ-Law 算法；
    // 11：A-Law 算法。
    // 注：仅在选择了 TDM 协议时才能使用压扩模式。
    uint32_t COMP:2;
    uint32_t :16; 
};

struct [[nodiscard]] R32_SAI_xFRCR{
    // 定义以 SCK 时钟周期数表示的音频帧长度：帧中
    // 的位数等于 FRL[7:0] + 1。
    // 音频帧中发送的位数必须大于或等于 8，否则音频
    // 模块将出现操作异常。数据大小为 8 位且在
    // R32_SAI_xSLOTR 寄存器的 NBSLOT[4:0]中只定义
    // 了一个 Slot（NBSLOT[3:0] = 0000）时便属于这
    // 种情况。
    // 在主模式下，如果使用主时钟（MCLK_x 引脚上提
    // 供），则帧长度应为 8 到 256 之间的一个等于 2 的
    // 几次幂的数。不使用主时钟（NODIV = 1）时，建
    // 议将帧长度编程为 8 到 256 之间的值。
    uint32_t FRL:8;

    // 帧同步有效电平长度：
    // 指定音频帧中 FS 信号的有效电平长度，以位时钟
    // 数（SCK） + 1（FSALL[6:0] + 1）表示。
    uint32_t FSALL:7;
    uint32_t :1;

    // 帧同步定义：
    // 1：FS 信号为 SOF 信号 + 通道识别信号；
    // 0：FS 信号为起始帧信号。
    // 此位置 1 时，R32_SAI_xSLOTR 寄存器中定义的
    // Slot 数必须为偶数。这意味着有半数 Slot 将用于
    // 左通道，其它 Slot 用于右通道（例如，对于 I2S
    // 或 MSB/LSB 对齐等协议，该位必须置 1）。
    uint32_t FSDEF:1;

    // 帧同步极性：
    // 1：FS 为高电平有效（上升沿）；
    // 0：FS 为低电平有效（下降沿）。
    uint32_t FSPOL:1;

    // 帧同步偏移：
    // 1：在 Slot 0 第一位的前一位上使能 FS；
    // 0：在 Slot 0 的第一位上使能 FS。
    uint32_t FSOFF:1;
    uint32_t :13;
};

struct [[nodiscard]] R32_SAI_xSLOTR{
    uint32_t FBOFF:5;
    uint32_t :1;
    uint32_t SOLTSSZ:1;
    uint32_t NBSLOT:4;
    uint32_t :4;
    uint32_t SLOTEN:16;
};

struct [[nodiscard]] R32_SAI_xINTENR{
    uint32_t OVRUDRIE:1;
    uint32_t MUTEDETIE:1;
    uint32_t WCKCFGIE:1;
    uint32_t FREQIE:1;
    uint32_t CNRDYIE:1;
    uint32_t AFSDETIE:1;
    uint32_t LFSDETIE:1;
    uint32_t :25;
};

struct [[nodiscard]] R32_SAI_xSR{
    uint32_t OVRUDR:1;
    uint32_t MUTEDET:1;
    uint32_t WCKCFG:1;

    // FIFO 请求标志：
    // 1：FIFO 请求读取或写入 R32_SAI_xDATAR；
    // 0：无 FIFO 请求。
    // 请求内容取决于音频模块的配置：
    // –如果模块配置为发送模式，则 FIFO 请求与向
    // R32_SAI_xDATAR 中写入相关。
    // –如果音频模块配置为接收模式，则 FIFO 请求与
    // 从 R32_SAI_xDATAR 中读取相关。
    // 注：写 1 清 0，写 0 无效。
    uint32_t FREQ:1;

    // 编解码器未就绪：
    // 1：外部 AC’97 编解码器未就绪；
    // 0：外部 AC’97 编解码器已就绪。
    // 注：（1）仅当在 R32_SAI_xCFGR1 寄存器中选择了
    // AC’97 音频模块并且音频模块配置为接收器模式
    // 时，才使用该位。
    // （2）写 1 清 0，写 0 无效。
    uint32_t CNRDY:1;
    uint32_t AFSDET:1;
    uint32_t LFSDET:1;
    uint32_t :9;

    // FIFO 阈值标志：
    // 如果将 SAI 模块配置为发送器：
    // 000：FIFO 为空；
    // 001：FIFO <= 1/4，但非空；
    // 010：1/4 < FIFO <= 1/2；
    // 011：1/2 < FIFO <= 3/4；
    // 100：3/4 < FIFO，但未满；
    // 101：FIFO 已满；
    // 其他：保留。
    // 如果将 SAI 模块配置为接收器：
    // 000：FIFO 为空；
    // 001：FIFO < 1/4，但非空；
    // 010：1/4 <= FIFO < 1/2；
    // 011：1/2 =< FIFO < 3/4；
    // 100：3/4 =< FIFO，但未满；
    // 101：FIFO 已满；
    // 其他：保留。
    uint32_t FLTH:3;
    uint32_t :13;
};

struct [[nodiscard]] R32_SAI_DATAR{
    uint32_t DATA;
};

}