#pragma once

#include <cstdint>
#include "core/io/regs.hpp"


#ifndef ASSERT_REG_IS_32BIT
#define ASSERT_REG_IS_32BIT(name) static_assert((sizeof(name) == 4),  #name " is not 4 bytes"); 
#endif

namespace ymd::ral::CH32{


struct R32_DFSDM_CHyCFGR1{
    // 通道 y 串行接口类型：
    // 1：下降沿选通数据 SPI；
    // 0：上升沿选通数据 SPI。
    // 只有在（R32_DFSDM_CHyCFGR1 寄存器中的）CHEN=0
    // 时，才能修改该值。
    uint32_t SITP:1;
    uint32_t :1;

    // 通道 y SPI 时钟选择：
    // 00：时钟来自外部 CKINy 输入——基于 SITP 的采样点；
    // 01：时钟来自内部 CKOUT 输出——基于 SITP 的采样点；
    // 10：时钟来自内部 CKOUT——在 CKOUT 每个第二个下降沿的采样点。
    // 用于连接外部 ΣΔ 调制器，调制器将时钟输入（来
    // 自 CKOUT）除以 2，来生成输出串行通信时钟（该
    // 输出时钟变化在各个时钟输入上升沿有效）。
    // 11：时钟来自内部 CKOUT 输出——在 CKOUT 每个
    // 第二个上升沿的采样点。
    // 用于连接外部 ΣΔ 调制器，调制器将时钟输入（来
    // 自 CKOUT）除以 2，来生成输出串行通信时钟（该
    // 输出时钟变化在各个时钟输入下降沿有效）。
    // 只有在（R32_DFSDM_CHyCFGR1 寄存器中）CHEN=0
    // 时，才能修改该值。
    uint32_t SPICKSEL:2;
    uint32_t :1;

    // 通道 y 短路检测器使能：
    // 1：输入通道 y 将持续受短路检测器保护；
    // 0：输入通道 y 将不受短路检测器保护。
    uint32_t SCDEN:1;

    // 通道 y 时钟缺失检测器使能：
    // 1：使能通道 y 时钟缺失检测器；
    // 0：禁止通道 y 时钟缺失检测器。
    uint32_t CKABEN:1;

    // 通道 y 使能：
    // 1：使能通道 y；
    // 0：禁止通道 y。
    // 如果使能了通道 y，则会根据给定通道设置开始接
    // 收串行数据。
    uint32_t CHEN:1;

    // 通道输入选择：
    // 1：通道输入取自下一通道（通道（y+1）取 2 的 模）的引脚；
    // 0：通道输入取自同一通道 y 的引脚。
    // 只有在（R32_DFSDM_CHyCFGR1 寄存器中）CHEN=0
    // 时，才能修改该值。
    uint32_t CHINSEL:1;
    uint32_t :3;

    // 通道 y 输入数据复用器：
    // 00：通道 y 输入数据取自外部串行输入，为 1 位
    // 值。R32_DFSDM_CHyDATINR 寄存器被写保护；
    // 01：通道 y 输入数据取自内部模数转换器 ADCy+1 
    // 输出寄存器更新，为 16 位值（若 ADCy+1 存在）。
    // ADC 输出数据被写入 R32_DFSDM_CHyDATINR 寄存
    // 器的 INDAT0[15:0]部分；
    // 10 ： 通 道 y 输 入 数 据 取 自 内 部
    // R32_DFSDM_CHyDATINR 寄存器（通过 CPU/DMA 直接
    // 写操作实现）。可写入一个或两个 16 位数据采样，
    // 具体取决于 DATPACK[1:0]位域设置；
    // 11：保留。
    // 只有在（R32_DFSDM_CHyCFGR1 寄存器中）CHEN=0
    // 时，才能修改该值。
    // 注：DATMPX[1:0]=1 仅适用于 ADC1 和 ADC2。
    uint32_t DATMPX:2;

    // R32_DFSDM_CHyDATINR 寄存器数据封装模式：
    // 00：标准：R32_DFSDM_CHyDATINR 寄存器中的输入
    // 数 据 仅 存 储 在 INDAT0[15:0] 中 。 要 清 空
    // R32_DFSDM_CHyDATINR 寄存器，必须由 DFSDM 滤波
    // 器从通道 y 中读取一个采样。
    // 01：交错：R32_DFSDM_CHyDATINR 寄存器中的输入
    // 数据存储为两个采样：
    // –第一个采样位于 INDAT0[15:0]中（已分配至通
    // 道 y）；
    // –第二个采样位于 INDAT1[15:0]中（已分配至通
    // 道 y）。
    // 要清空 R32_DFSDM_CHyDATINR 寄存器，必须由数
    // 字滤波器从通道 y 中读取两个采样（INDAT0[15:0]
    // 部分被读为第一个采样，而 INDAT1[15:0]部分则
    // 被读为下一个采样）。
    // 10：双通道：R32_DFSDM_CHyDATINR 寄存器中的输
    // 入数据存储为两个采样：
    // –第一个采样位于 INDAT0[15:0]中（已分配至通
    // 道 y）；
    // –第二个样采样于 INDAT1[15:0]中（已分配至通
    // 道 y+1）。
    // 要清空 R32_DFSDM_CHyDATINR 寄存器，必须由数
    // 字滤波器从通道 y 中读取第一个样本，且必须由
    // 另一数字滤波器从通道 y+1 中读取第二个样本。
    // 只有在通道数为偶数（y=0）时才支持双通道模式，
    // 对 于 通 道 数 为 奇 数 （ y=1 ） 的 情 况 ，
    // R32_DFSDM_CHyDATINR 被写保护。如果偶数通道被
    // 设为双通道模式，则下一奇数通道必须进入标准
    // 模式（DATPACK[1:0]=0），以便与偶数通道正确配
    // 合。
    // 11：保留，只有在（R32_DFSDM_CHyCFGR1 寄存器
    // 中）CHEN=0 时，才能修改该值。
    uint32_t DATPACK:2;

    // 输出串行时钟分频器：
    // 0：禁止输出时钟生成（CKOUT 信号被设为低电平
    // 状态）；
    // 1～255：为 CKOUT 信号定义串行时钟输出的系统
    // 时钟分频，范围为 2～256（分频器值=CKOUTDIV+ 
    // 1）。
    // CKOUTDIV 还定义了时钟缺失检测的阈值。
    // 只 有 在 （ R32_DFSDM_CH0CFGR1 寄 存 器 中 ）
    // DFSDMEN=0 时，才能修改该值。
    // 如果 DFSDMEN=0（R32_DFSDM_CH0CFGR1 寄存器中），
    // 则 CKOUT 信号被置为低电平状态（在 DFSDMEN=0
    // 后的一个 DFSDM 时钟周期内执行设置）。
    // 注：CKOUTDIV 仅存在于 R32_DFSDM_CH0CFGR1 寄存
    // 器（通道 y=0）中。
    uint32_t CKOUTDIV:8;
    uint32_t :6;

    // 输出串行时钟源选择：
    // 1：输出时钟的源来自音频时钟；
    // 0：输出时钟的源来自外设时钟。
    // 只 有 在 （ R32_DFSDM_CH0CFGR1 寄 存 器 中 ）
    // DFSDMEN=0 时，才能修改该值。
    // 注：CKOUTSRC 仅存在于 R32_DFSDM_CH0CFGR1 寄存
    // 器（通道 y=0）中。
    uint32_t CKOUTSRC:1;

    // DFSDM 接口全局使能：
    // 1：使能 DFSDM 接口；
    // 0：禁止 DFSDM 接口。
    // 如果已使能 DFSDM 接口，则该接口将根据使能的
    // y 通 道 和 使 能 的 x 滤 波 器 设 置
    // （ R32_DFSDM_CHyCFGR1 的 CHEN 位 和
    // R32_DFSDM_FLTxCR1 的 DFEN 位）进行工作。通过
    // 设置 DFSDMEN=0 清除数据：
    // –所有寄存器 R32_DFSDM_FLTxISR 均被设为复位
    // 状态（x=0、1）；
    // –所有寄存器 R32_DFSDM_FLTxAWSR 均被设为复位
    // 状态（x=0、1）。
    // 注：DFSDMEN 仅存在于 R32_DFSDM_CH0CFGR1 寄存
    // 器（通道 y=0）中。
    uint32_t DFSDMEN:1;
};

struct R32_DFSDM_CHyCFGR2{
    uint32_t :3;
    uint32_t DRRBS:5;
    uint32_t OFFSET:24;
};

struct R32_DFSDM_CHyAWSCDR{
    uint32_t SCDT:8;
    uint32_t :4;
    uint32_t BKSCD:2;
    uint32_t :2;
    uint32_t AWFOSR:5;
    uint32_t :1;
    uint32_t AWFORD:2;
    uint32_t :8;
};

struct R32_DFSDM_CHyDATAR{
    uint32_t DATA:16;
    uint32_t :16;
};

}