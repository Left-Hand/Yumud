#pragma once

#include "dma.hpp"


// DMA1 在使用时需注意，对于批号第五位小于 3，DMA1 所有通道的 DMA 访问地址不得超过 64K 边
// 界，例如-CH32V307VCT6 配置 RAM 为 128K，此时配置 DAM1 需注意 DMA 源地址+传输数据数目，或 DMA
// 目的地址+传输数据只能在 0-64K，或 64K-128K 区域，不可出现 63K-65K 类似情况。对于批号第五位
// 等于 3，DAM1 通道 2，3，4，5，DMA 访问地址不得超过 128K 边界，DMA1 通道 1，6，7DMA 访问地址依
// 然不得超过 64K 边界。DMA2 使用上没有限制。

extern"C"{
#ifdef DMA1_PRESENT
    __interrupt void DMA1_Channel1_IRQHandler(void);
    __interrupt void DMA1_Channel2_IRQHandler(void);
    __interrupt void DMA1_Channel3_IRQHandler(void);
    __interrupt void DMA1_Channel4_IRQHandler(void);
    __interrupt void DMA1_Channel5_IRQHandler(void);
    __interrupt void DMA1_Channel6_IRQHandler(void);
    __interrupt void DMA1_Channel7_IRQHandler(void);
    __interrupt void DMA1_Channel8_IRQHandler(void);
#endif

#ifdef DMA2_PRESENT
    __interrupt void DMA2_Channel1_IRQHandler(void);
    __interrupt void DMA2_Channel2_IRQHandler(void);
    __interrupt void DMA2_Channel3_IRQHandler(void);
    __interrupt void DMA2_Channel4_IRQHandler(void);
    __interrupt void DMA2_Channel5_IRQHandler(void);
    __interrupt void DMA2_Channel6_IRQHandler(void);
    __interrupt void DMA2_Channel7_IRQHandler(void);
    __interrupt void DMA2_Channel8_IRQHandler(void);
    __interrupt void DMA2_Channel9_IRQHandler(void);
    __interrupt void DMA2_Channel10_IRQHandler(void);
    __interrupt void DMA2_Channel11_IRQHandler(void);
#endif


}

namespace ymd::hal{

#ifdef DMA1_PRESENT
extern DmaChannel dma1_ch1;
extern DmaChannel dma1_ch2;
extern DmaChannel dma1_ch3;
extern DmaChannel dma1_ch4;
extern DmaChannel dma1_ch5;
extern DmaChannel dma1_ch6;
extern DmaChannel dma1_ch7;
extern DmaChannel dma1_ch8;
#endif

#ifdef DMA2_PRESENT
extern DmaChannel dma2_ch1;
extern DmaChannel dma2_ch2;
extern DmaChannel dma2_ch3;
extern DmaChannel dma2_ch4;
extern DmaChannel dma2_ch5;
extern DmaChannel dma2_ch6;
extern DmaChannel dma2_ch7;
extern DmaChannel dma2_ch8;
extern DmaChannel dma2_ch9;
extern DmaChannel dma2_ch10;
extern DmaChannel dma2_ch11;
#endif
}