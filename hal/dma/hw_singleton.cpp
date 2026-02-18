#include "hw_singleton.hpp"
#include "core/sdk.hpp"
#include "ral/ch32/ch32_common_dma_def.hpp"


using namespace ymd;
using namespace ymd::hal;


namespace ymd::hal{


#ifdef DMA1_PRESENT
DmaChannel dma1_ch1{DMA1_Channel1};
DmaChannel dma1_ch2{DMA1_Channel2};
DmaChannel dma1_ch3{DMA1_Channel3};
DmaChannel dma1_ch4{DMA1_Channel4};
DmaChannel dma1_ch5{DMA1_Channel5};
DmaChannel dma1_ch6{DMA1_Channel6};
DmaChannel dma1_ch7{DMA1_Channel7};

//DMA1CH8只有部分芯片有
#if defined(DMA1_Channel8)
DmaChannel dma1_ch8{DMA1_Channel8};
#endif
#endif

#ifdef DMA2_PRESENT
DmaChannel dma2_ch1{DMA2_Channel1};
DmaChannel dma2_ch2{DMA2_Channel2};
DmaChannel dma2_ch3{DMA2_Channel3};
DmaChannel dma2_ch4{DMA2_Channel4};
DmaChannel dma2_ch5{DMA2_Channel5};
DmaChannel dma2_ch6{DMA2_Channel6};
DmaChannel dma2_ch7{DMA2_Channel7};
DmaChannel dma2_ch8{DMA2_Channel8};
DmaChannel dma2_ch9{DMA2_Channel9};
DmaChannel dma2_ch10{DMA2_Channel10};
DmaChannel dma2_ch11{DMA2_Channel11};
#endif
}


#define NAME_OF_DMA_XY(NUM_DMA,NUM_CH) dma##NUM_DMA##_ch##NUM_CH

#ifdef DMA1_PRESENT

#define DMA1_Inst reinterpret_cast<ral::DMA1_Def *>(DMA1)

#define DMA1_IT_TEMPLATE(NUM_CH)\
__interrupt void DMA1##_Channel##NUM_CH##_IRQHandler(void){\
    if(DMA1_Inst->get_transfer_done_flag<NUM_CH>()){\
        NAME_OF_DMA_XY(1,NUM_CH).accept_interrupt(DmaEvent::TransferComplete);\
        DMA1_Inst->clear_transfer_done_flag<NUM_CH>();\
    }else if(DMA1_Inst->get_transfer_onhalf_flag<NUM_CH>()){\
        NAME_OF_DMA_XY(1,NUM_CH).accept_interrupt(DmaEvent::HalfTransfer);\
        DMA1_Inst->clear_transfer_onhalf_flag<NUM_CH>();\
    }\
}\


DMA1_IT_TEMPLATE(1);
DMA1_IT_TEMPLATE(2);
DMA1_IT_TEMPLATE(3);
DMA1_IT_TEMPLATE(4);
DMA1_IT_TEMPLATE(5);
DMA1_IT_TEMPLATE(6);
DMA1_IT_TEMPLATE(7);

#if defined(DMA1_Channel8)
DMA1_IT_TEMPLATE(8);
#endif

#endif

#ifdef DMA2_PRESENT

#define DMA2_Inst reinterpret_cast<ral::DMA2_Def *>(DMA2)

#define DMA2_IT_TEMPLATE(NUM_CH)\
__interrupt void DMA2##_Channel##NUM_CH##_IRQHandler(void){\
    if(DMA2_Inst->get_transfer_done_flag<NUM_CH>()){\
        NAME_OF_DMA_XY(2,NUM_CH).accept_interrupt(DmaEvent::TransferComplete);\
        DMA2_Inst->clear_transfer_done_flag<NUM_CH>();\
    }else if(DMA2_Inst->get_transfer_onhalf_flag<NUM_CH>()){\
        NAME_OF_DMA_XY(2,NUM_CH).accept_interrupt(DmaEvent::HalfTransfer);\
        DMA2_Inst->clear_transfer_onhalf_flag<NUM_CH>();\
    }\
}\

DMA2_IT_TEMPLATE(1);
DMA2_IT_TEMPLATE(2);
DMA2_IT_TEMPLATE(3);
DMA2_IT_TEMPLATE(4);
DMA2_IT_TEMPLATE(5);
DMA2_IT_TEMPLATE(6);
DMA2_IT_TEMPLATE(7);
DMA2_IT_TEMPLATE(8);
DMA2_IT_TEMPLATE(9);
DMA2_IT_TEMPLATE(10);
DMA2_IT_TEMPLATE(11);
#endif