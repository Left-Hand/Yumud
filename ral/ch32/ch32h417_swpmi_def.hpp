#pragma once

#include <cstdint>
#include "core/io/regs.hpp"


#ifndef ASSERT_REG_IS_32BIT
#define ASSERT_REG_IS_32BIT(name) static_assert((sizeof(name) == 4),  #name " is not 4 bytes"); 
#endif

namespace ymd::ral::CH32{

struct [[nodiscard]] R32_SWPMI_CR{
    // 接收 DMA 使能
    uint32_t RXDMA:1;

    // 发送 DMA 使能：
    // 1：针对发送使能 DMA；
    // 0：针对发送禁止 DMA。
    // 注：如果将已发送帧的有效负载大小指定为 0x00 
    // （在帧的第一个字的 TDR 最低有效字节中），TXDMA 
    // 将 自 动 清 零 。 TXDMA 在 发 生 下 溢 事 件 时
    // （R32_SWP_ISR 寄存器的 TXUNRF 标志置 1 时）也
    // 会自动清零。
    uint32_t TXDMA:1;

    // 接收缓冲模式：
    // 1：针对接收将 SWPMI 配置为多软件缓冲区模式；
    // 0：针对接收将 SWPMI 配置为单软件缓冲区模式。
    // 注：SWPACT 位置 1 时，该位无法写入。
    uint32_t RXMODE:1;

    // 发送缓冲模式：
    // 1：针对发送将 SWPMI 配置为多软件缓冲区模式；
    // 0：针对发送将 SWPMI 配置为单软件缓冲区模式。
    // 注：SWPACT 位置 1 时，该位无法写入。
    uint32_t TXMODE:1;

    // 回送模式使能
    uint32_t LPBK:1;

    // 单线协议主接口激活：
    // 1：SWPMI_IO 被释放，SWP 总线切换到“已挂起”
    // 状态；
    // 0：SWPMI_IO 被下拉至地，SWP 总线切换到“已禁
    // 用”状态。
    // 将 SWPACT 位置 1，必须先清零 DEACT 位。
    uint32_t SWPACT:1;
    uint32_t :4;

    // 单线协议主接口禁用，用于请求 SWP“已禁用”状
    // 态。除了可能传入的“从器件恢复”状态会使 SWP 
    // 保持为“已激活”状态外，将该位置 1 与清零
    // SWPACT 的作用相同。
    uint32_t DEACT:1;

    // 单线协议主收发器使能：
    // 1：SWPMI_IO 收发器由 SWPMI 控制；
    // 0：SPWMI_IO 引脚由 GPIO 控制器控制。
    uint32_t SWPTEN:1;
    uint32_t :20;
};ASSERT_REG_IS_32BIT(R32_SWPMI_CR)

struct [[nodiscard]] R32_SWPMI_BRR{
    // 比特率预分频器，必须按照以下公式，在考虑到
    // RCC（复位和时钟控制）中编程的 FHCLK的情况下编
    // 程该位域以设置 SWP 总线比特率：
    // FSWP = FHCLK / （（BR[7:0]+1）x4）
    // 注：编程的比特率必须处于以下范围内：100 
    // kbit/s～2Mbit/s。
    // R32_SWPMI_CR 寄存器中的 SWPACT 位置 1 时，不能
    // 写入 BR[7:0]。
    uint32_t BR:8;
    uint32_t :24;
};ASSERT_REG_IS_32BIT(R32_SWPMI_BRR)

struct [[nodiscard]] R32_SWPMI_ISR {
    // 接收缓冲区已满标志：当 R32_SWPMI_RDR 中包含
    // 接收帧的最后一个字时，该标志由硬件置 1。该标
    // 志由软件清零，方法是向 R32_SWPMI_ICR 寄存器
    // 中的 CRXBFF 位写入 1。
    // 1：接收帧的最后一个字已传到 R32_SWPMI_RDR；
    // 0：接收帧的最后一个字尚未传到 R32_SWPMI_RDR。
    uint32_t RXBFF:1;

    // 发送缓冲区为空标志：该标志由硬件置 1，用于指
    // 示无需进一步进行 R32_SWPMI_TDR 更新来完成当
    // 前 帧 发 送 。 该 位 由 软 件 清 零 ， 方 法 是 向
    // R32_SWPMI_ICR 寄存器中的 CTXBEF 位写入 1。
    // 1：帧发送缓冲区已清空；
    // 0：帧发送缓冲区尚未清空。
    uint32_t TXBFF:1;

    // 接收 CRC 错误标志：该标志由硬件置 1，用于指示
    // 接收到的帧中存在 CRC 错误。它与 RXBFF 标志同
    // 步置 1。该位由软件清零，方法是向 R32_SWPMI_ICR
    // 寄存器中的 CRXBERF 位写入 1。
    // 1：在接收期间检测到 CRC 错误；
    // 0：接收期间无 CRC 错误。
    uint32_t RXBERF:1;

    // 接收上溢错误标志：该标志由硬件置 1，用于指示
    // 在有效负载接收期间发生上溢，即软件或 DMA 未
    // 及时对 R32_SWPMI_RDR 进行读操作。该标志由软
    // 件清零，方法是向 R32_SWPMI_ICR 寄存器中的
    // CRXOVRF 位写入 1。
    // 1：在接收期间检测到上溢错误；
    // 0：接收期间无上溢错误。
    uint32_t RX0VRF:1;

    // 发送下溢错误标志：该标志由硬件置 1，用于指示
    // 在有效负载发送期间发生下溢，即软件或 DMA 未
    // 及时对 R32_SWPMI_TDR 进行写操作。该标志由软
    // 件清零，方法是向 R32_SWPMI_ICR 寄存器中的
    // CTXUNRF 位写入 1。
    // 1：在发送期间检测到下溢错误；
    // 0：发送期间无下溢错误。
    uint32_t TXUNRF:1;

    // 接收数据寄存器非空：
    // 1：R32_SWPMI_RDR 寄存器中接收到的数据已准备
    // 0：R32_SWPMI_RDR 寄存器中未接收到数据。
    uint32_t RXNE:1;

    // 发送数据寄存器为空：
    // 1：已发送写入到发送数据寄存器 R32_SWPMI_TDR
    // 中的数据，可再次对 R32_SWPMI_TDR 进行写操作；
    // 0 ： 尚 未 发 送 写 入 到 发 送 数 据 寄 存 器
    // R32_SWPMI_TDR 中的数据。
    uint32_t TXE:1;

    // 传输完成标志：当发送和接收均已完成并且 SWP 
    // 切换到“已挂起”状态时，该标志将立即由硬件
    // 置 1。该位由软件清零，方法是向 R32_SWPMI_ICR
    // 寄存器中的 CTCF 位写入 1。
    // 1：发送和接收均已完成并且 SWP 切换到“已挂
    // 起”状态；
    // 0：发送或接收未完成。
    uint32_t TCF:1;

    // 从器件恢复标志：该位由硬件置 1，用于指示检测
    // 到“从器件恢复”状态。该位由软件清零，方法是
    // 向 R32_SWPMI_ICR 寄存器中的 CSRF 位写入 1。
    // 1：在 SWP 总线处于“已挂起”状态时检测到“从
    // 器件恢复”状态；
    // 0：未检测到“从器件恢复”状态。
    uint32_t SRF:1;

    // 挂起标志：
    // 1：SWP 总线处于“已挂起”或“已禁用”状态；
    // 0：SWP 总线处于“已激活”状态。
    uint32_t SUSP:1;

    // 已禁用标志：
    // 1：SWP 总线处于“已禁用”状态；
    // 0：SWP 总线处于“已激活”状态或“已挂起”状
    // 态。
    uint32_t DEACTF:1;

    // 收发器就绪标志：当收发器就绪时，该位立即由硬
    // 件置 1。通过将 R32_SWPMI_CR 寄存器中的 SWPTEN
    // 位置 1 以使能 SWPMI_IO 收发器后，软件必须等待
    // 此标志置 1 后再将 SWPACT 位置 1 以激活 SWP 总
    // 线。
    // 1：收发器已就绪；
    // 0：收发器未就绪。
    uint32_t RDYF:1;
    uint32_t :20;
};ASSERT_REG_IS_32BIT(R32_SWPMI_ISR)

struct [[nodiscard]] R32_SWPMI_ICR{
    uint32_t CRXBFF:1;
    uint32_t CTXBEF:1;
    uint32_t CRXBERF:1;
    uint32_t CRXOVRF:1;
    uint32_t CTXUNRF:1;
    uint32_t :2;
    uint32_t CTCF:1;
    uint32_t CSRF:1;
    uint32_t :2;
    uint32_t :20;
};ASSERT_REG_IS_32BIT(R32_SWPMI_ICR)

struct [[nodiscard]] R32_SWPMI_IER{
    uint32_t RXBFIE:1;
    uint32_t TXBEIE:1;
    uint32_t RXBERIE:1;
    uint32_t RXOVRIE:1;
    uint32_t TXUNRIE:1;
    uint32_t RIE:1;
    uint32_t TIE:1;
    uint32_t TCIE:1;
    uint32_t SRIE:1;
    uint32_t :2;
    uint32_t RDYIE:1;
    uint32_t :20;
}; ASSERT_REG_IS_32BIT(R32_SWPMI_IER)

struct [[nodiscard]] R32_SWPMI_RFL{
    uint32_t RFL:5;
    uint32_t :27;
}; ASSERT_REG_IS_32BIT(R32_SWPMI_RFL)

struct [[nodiscard]] R32_SWPMI_TDR{
    uint32_t TD:32;
}; ASSERT_REG_IS_32BIT(R32_SWPMI_TDR)

struct [[nodiscard]] R32_SWPMI_RDR{
    uint32_t RD:32;
}; ASSERT_REG_IS_32BIT(R32_SWPMI_RDR)

struct [[nodiscard]] R32_SVPMI_OR{
    uint32_t SWP_TBYP:1;
    uint32_t :31;
}; ASSERT_REG_IS_32BIT(R32_SWPMI_OR)


}