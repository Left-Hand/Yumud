#pragma once

#include <cstdint>
#include "core/io/regs.hpp"


#ifndef ASSERT_REG_IS_32BIT
#define ASSERT_REG_IS_32BIT(name) static_assert((sizeof(name) == 4),  #name " is not 4 bytes"); 
#endif

namespace ymd::ral::CH32{

struct [[nodiscard]] R32_QSPIx_CR{
    uint32_t EN:1;
    uint32_t ABORT:1;
    uint32_t DMAEN:1;
    uint32_t TCEN:1;
    uint32_t :1;
    uint32_t START:1;
    uint32_t DFM:1;
    uint32_t FSEL:1;

    // FIFO 阈值级别：
    // 定义在间接模式下 FIFO 中将导致 FIFO 阈值标志
    // （FTF，R32_QSPIx_SR[2]）置 1 的字节数阈值。
    // 在间接写入模式下（FMODE=00）：
    // 00000：如果 FIFO 中存在 1 个或更多空闲字节可
    // 供写入，则 FTF 置 1；
    // 00001：如果 FIFO 中存在 2 个或更多空闲字节可
    // 供写入，则 FTF 置 1；
    // ... 
    // 11111：如果 FIFO 中存在 32 个空闲字节可供写
    // 入，则 FTF 置 1。
    // 在间接读取模式下（FMODE=01）：
    // 00000：如果 FIFO 中存在 1 个或更多有效字节可
    // 供读取，则 FTF 置 1；
    // 00001：如果 FIFO 中存在 2 个或更多有效字节可
    // 供读取，则 FTF 置 1；
    // ... 
    // 11111：如果 FIFO 中存在 32 个有效字节可供读
    // 取，则 FTF 置 1。
    uint32_t FTHRES:5;

    // SIOX 使能：
    // 1：4 线 IO 输出使能；
    // 0：4 线 IO 输出关闭。
    uint32_t SIOXEN:1;
    uint32_t :2;
    uint32_t TEIE:1;
    uint32_t TCIE:1;
    uint32_t FTIE:1;
    uint32_t SMIE:1;
    uint32_t TOIE:1;
    uint32_t :1;
    uint32_t APMS:1;

    // 轮询匹配模式：
    // 1：OR 匹配模式。如果从 FLASH 接收的任意一个未
    // 屏蔽位与匹配寄存器中的对应位相匹配，则 SMF 置
    // 1；
    // 0：AND 匹配模式。如果从 FLASH 接收的所有未屏
    // 蔽位均与匹配寄存器中的对应位相匹配，则 SMF 置
    // 1。
    // 仅可在 BUSY=0 时修改该位。
    uint32_t PMM:1;

    // 时钟预分频器：
    // 该字段定义基于 HB 总线时钟生成 SCK 所用的分频
    // 系数（值+1）。
    // 0x0：FSCK=FHCLK，HCLK 时钟直接用作 QSPI SCK（预
    // 分频器被旁路）；
    // 0x1：FSCK=FHCLK/2；
    // 0x2：FSCK=FHCLK/3；
    // ... 
    // 0xFF：FSCK=FHCLK/256。
    // 对于奇数时钟分频系数，SCK 的占空比并非 50%。
    // 时钟信号的低电平持续时间比高电平持续时间多
    // 一个周期。
    // 仅可在 BUSY=0 时修改该字段。
    uint32_t PRESCALER:8;
};

struct [[nodiscard]] R32_QSPIx_DCR{
    uint32_t CKMODE:1;
    uint32_t :7;
    uint32_t CSHT:3;
    uint32_t :5;
    uint32_t FSIZE:5;
    uint32_t :11;
};

struct [[nodiscard]] R32_QSPIx_SR{
    uint32_t TEF:1;
    uint32_t TCF:1;
    uint32_t FTF:1;
    uint32_t SMF:1;
    uint32_t TOF:1;
    uint32_t BUSY:1;
    uint32_t :2;
    uint32_t FLEVEL:6;
    uint32_t :18;
};

struct [[nodiscard]] R32_QSPIx_FCR{
    uint32_t CTEF:1;
    uint32_t CTCF:1;
    uint32_t :1;
    uint32_t CSMF:1;
    uint32_t CTOF:1;
    uint32_t :27;
};

struct [[nodiscard]] R32_QSPIx_DLR{
    // 数据长度：
    // 在间接模式和状态轮询模式下待检索的数据数量
    // （值+1）。对状态轮询模式应使用不大于 3 的值
    // （表示 4 字节）。
    // 在间接模式下，所有位置 1 表示未定义长度，QSPI
    // 将继续传输数据直到到达由 FSIZE 定义的存储器
    // 末尾。
    // 0x0000_0000：传输 1 个字节；
    // 0x0000_0001：传输 2 个字节；
    // 0x0000_0002：传输 3 个字节；
    // 0x0000_0003：传输 4 个字节；
    // ... 
    // 0xFFFF_FFFD：传输 4，294，967，294（4G-2）个
    // 字节；
    // 0xFFFF_FFFE：传输 4，294，967，295（4G-1）个
    // 字节；
    // 0xFFFF_FFFF：未定义长度--传输所有字节直到到
    // 达 由 FSIZE 定 义 的 FLASH 的 结 尾 。 如 果
    // FSIZE=0x1F，则读取无限继续下去。
    // 在双闪存模式（DFM=1）下，即使该位写入 0，DL[0]
    // 也始终保持为 1，因此保证了每次访问均传输偶数
    // 个字节。
    // 该字段在内存映射模式（FMODE=10）下不起作用；
    // 仅可在 BUSY=0 时写入该字段。
    uint32_t DL;
};

struct [[nodiscard]] R32_QSPIx_CCR{
    uint32_t INSTRUCTION:8;
    uint32_t IMODE:2;

    // 地址模式：
    // 00：无地址；
    // 01：单线传输地址；
    // 10：双线传输地址；
    // 11：四线传输地址。
    uint32_t ADMODE:2;

    // 地址长度：
    // 00：8 位地址；
    // 01：16 位地址；
    // 10：24 位地址；
    // 11：32 位地址。
    // 仅可在 BUSY=0 时写入该字段。
    uint32_t ADSIZE:2;

    // 交替字节模式：
    // 00：无交替字节；
    // 01：单线传输交替字节；
    // 10：双线传输交替字节；
    // 11：四线传输交替字节。
    // 仅可在 BUSY=0 时写入该字段。
    uint32_t ABMODE:2;
    uint32_t ABSIZE:2;
    uint32_t DCYC:5;
    uint32_t :1;
    uint32_t DMODE:2;
    uint32_t FMODE:2;
    uint32_t SIOO:1;
    uint32_t :3;
};


struct [[nodiscard]] R32_QSPIx_AR{
    uint32_t ADDR;
};


struct [[nodiscard]] R32_QSPIx_ABR{
    uint32_t ALTERNATE;
};

struct [[nodiscard]] R32_QSPIx_DR{
    uint32_t DATA;
};


struct [[nodiscard]] R32_QSPIx_PSMKR{
    // 数据：
    // 指定要与外部 SPI 设备交换的数据。
    // 在间接写入模式下，写入该寄存器的数据在数据
    // 阶段发送到 FLASH，在此之前则存储于 FIFO。如
    // 果 FIFO 太满，将暂停写入，直到 FIFO 具有足够
    // 的空间接受要写入的数据才继续。
    // 在间接模式下，读取该寄存器可获得（通过 FIFO）
    // 已从 FLASH 接收的数据。如果 FIFO 所含字节数比
    // 读取操作要求的字节数少并且 BUSY=1，将暂停读
    // 取，直到足够的数据出现或传输完成（不分先后）
    // 才继续。
    // 在自动轮询模式下，该寄存器包含最后从 FLASH 
    // 读取的数据（未进行屏蔽）。
    // 支持对该寄存器进行字、半字以及字节访问。在间
    // 接写入模式下，字节写入将在 FIFO 中增加 1 个字
    // 节，半字写入增加 2 个，而字写入则增加 4 个。
    // 类似地，在间接读取模式下，字节读取将擦除
    // FIFO 中的 1 个字节，半字读取擦除 2 个，而字读
    // 取则擦除 4 个。
    // 间接模式下的访问必须与此寄存器的最低位对
    // 齐：字节读取必须读取 DATA[7:0]而半字读取必须
    // 读取 DATA[15:0]。
    uint32_t MASK;
};

struct [[nodiscard]] R32_QSPIx_PSMAR{
    uint32_t MATCH;
};

struct [[nodiscard]] R32_QSPIx_PIR{
    uint32_t INTERVAL:16;
    uint32_t :16;
};


struct [[nodiscard]] R32_QSPIx_LPTR{
    uint32_t TIMEOUT:16;
    uint32_t :16;
};


}