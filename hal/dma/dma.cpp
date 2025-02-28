#include "dma.hpp"


using namespace ymd;

namespace ymd{
#ifdef ENABLE_DMA1
DmaChannel dma1Ch1{DMA1_Channel1};
DmaChannel dma1Ch2{DMA1_Channel2};
DmaChannel dma1Ch3{DMA1_Channel3};
DmaChannel dma1Ch4{DMA1_Channel4};
DmaChannel dma1Ch5{DMA1_Channel5};
DmaChannel dma1Ch6{DMA1_Channel6};
DmaChannel dma1Ch7{DMA1_Channel7};
#endif

#ifdef ENABLE_DMA2
DmaChannel dma2Ch1{DMA2_Channel1};
DmaChannel dma2Ch2{DMA2_Channel2};
DmaChannel dma2Ch3{DMA2_Channel3};
DmaChannel dma2Ch4{DMA2_Channel4};
DmaChannel dma2Ch5{DMA2_Channel5};
DmaChannel dma2Ch6{DMA2_Channel6};
DmaChannel dma2Ch7{DMA2_Channel7};
DmaChannel dma2Ch8{DMA2_Channel8};
DmaChannel dma2Ch9{DMA2_Channel9};
DmaChannel dma2Ch10{DMA2_Channel10};
DmaChannel dma2Ch11{DMA2_Channel11};
#endif
}

[[maybe_unused]] static FlagStatus PL_DMA_GetFlagStatus(uint32_t DMAy_FLAG)
{
    FlagStatus bitstatus = RESET;
    uint32_t   tmpreg = 0;

    tmpreg = DMA1->INTFR;

    if((tmpreg & DMAy_FLAG) != (uint32_t)RESET)
    {
        bitstatus = SET;
    }
    else
    {
        bitstatus = RESET;
    }

    return bitstatus;
}


#define NAME_OF_DMA_XY(x,y) dma##x##Ch##y

#define DMA_IT_TEMPLATE(x,y)\
__interrupt void DMA##x##_Channel##y##_IRQHandler(void){\
    if(DMA_GetFlagStatus(DMA##x##_IT_TC##y)){\
        NAME_OF_DMA_XY(x,y).onTransmitDoneInterrupt();\
        DMA_ClearFlag(DMA##x##_IT_TC##y);\
    }else if(DMA_GetFlagStatus(DMA##x##_IT_HT##y)){\
        NAME_OF_DMA_XY(x,y).onTransmitHalfInterrupt();\
        DMA_ClearFlag(DMA##x##_IT_HT##y);\
    }\
}\


#ifdef ENABLE_DMA1
DMA_IT_TEMPLATE(1,1);
DMA_IT_TEMPLATE(1,2);
DMA_IT_TEMPLATE(1,3);
DMA_IT_TEMPLATE(1,4);
DMA_IT_TEMPLATE(1,5);
DMA_IT_TEMPLATE(1,6);
DMA_IT_TEMPLATE(1,7);
#endif

#ifdef ENABLE_DMA2
DMA_IT_TEMPLATE(2,1);
DMA_IT_TEMPLATE(2,2);
DMA_IT_TEMPLATE(2,3);
DMA_IT_TEMPLATE(2,4);
DMA_IT_TEMPLATE(2,5);
DMA_IT_TEMPLATE(2,6);
DMA_IT_TEMPLATE(2,7);
DMA_IT_TEMPLATE(2,8);
DMA_IT_TEMPLATE(2,9);
DMA_IT_TEMPLATE(2,10);
DMA_IT_TEMPLATE(2,11);
#endif


void DmaChannel::enableRcc(bool en){

    #ifdef ENABLE_DMA2
    if(instance < DMA2_Channel1){
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, en);
    }else{
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, en);
    }

    #else
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, en);
    #endif
}


void DmaChannel::start(void * dst, const void * src, const size_t size){

    if(dstIsPeriph(mode_)){
        instance -> PADDR = (uint32_t)dst;
        instance -> MADDR = (uint32_t)src;
    }else{
        instance -> PADDR = (uint32_t)src;
        instance -> MADDR = (uint32_t)dst;
    }
    instance -> CNTR = size;
    start();
}


void DmaChannel::init(const Mode mode,const Priority priority){
    enableRcc(true);
    mode_ = mode;
    DMA_InitTypeDef DMA_InitStructure = {0};
    // DMA_DeInit(instance);

    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;

    switch(mode){
        case Mode::toMemCircular:
            DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
        case Mode::toMem:
            DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)NULL;
            DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)NULL;
            DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
            DMA_InitStructure.DMA_BufferSize = 0;
            DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
            DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
            DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
            break;
        case Mode::toPeriphCircular:
            DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
        case Mode::toPeriph:
            DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)NULL;
            DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)NULL;
            DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
            DMA_InitStructure.DMA_BufferSize = 0;
            DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
            DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
            DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
            break;
        case Mode::synergyCircular:
            DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
        case Mode::synergy:
            DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)nullptr;
            DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)nullptr;
            DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
            DMA_InitStructure.DMA_BufferSize = 0;
            DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
            DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
            DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;
            break;
        case Mode::distributeCircular:
            DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
        case Mode::distribute:
            DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)nullptr;
            DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)nullptr;
            DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
            DMA_InitStructure.DMA_BufferSize = 0;
            DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
            DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
            DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;
            break;
        case Mode::automatic:
            break;
    }

    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;

    DMA_InitStructure.DMA_Priority = (uint32_t)priority;

    DMA_Init(instance, &DMA_InitStructure);
}

void DmaChannel::enableIt(const NvicPriority _priority, const bool en){
    IRQn irq = IRQn_Type::Software_IRQn;
    switch(dma_index){
        case 1:
            irq = (IRQn)((int)DMA1_Channel1_IRQn + ((int)(DMA1_Channel2_IRQn - DMA1_Channel1_IRQn) * (channel_index - 1)));
            break;
        #ifdef ENABLE_DMA2
        case 2:
            if(channel_index <= 5){
                irq = (IRQn)((int)DMA2_Channel1_IRQn + ((int)(DMA2_Channel2_IRQn - DMA2_Channel1_IRQn) * (channel_index - 1)));
            }else{
                irq = (IRQn)((int)DMA2_Channel6_IRQn + ((int)(DMA2_Channel7_IRQn - DMA2_Channel6_IRQn) * (channel_index - 6)));
            }
            break;
        #endif
    }

    NvicPriority::enable(_priority, IRQn(irq), en);
}

void DmaChannel::onTransmitHalfInterrupt(){
    EXECUTE(half_cb_);
}

void DmaChannel::onTransmitDoneInterrupt(){
    EXECUTE(done_cb_);
}
