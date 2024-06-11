#include "dma.hpp"

#define DMA_DONE_CB(x,y) dma##x##_ch##y##_done_cb
#define DMA_HALF_CB(x,y) dma##x##_ch##y##_half_cb


#define DMA_IT_TEMPLATE(x,y)\
static DmaChannel::Callback DMA_DONE_CB(x,y);\
static DmaChannel::Callback DMA_HALF_CB(x,y);\
__interrupt void DMA##x##_Channel##y##_IRQHandler(void){\
    if(DMA_GetFlagStatus(DMA##x##_IT_TC##y)){\
        EXECUTE(DMA_DONE_CB(x,y));\
        DMA_ClearFlag(DMA##x##_IT_TC##y);\
    }else if(DMA_GetFlagStatus(DMA##x##_IT_HT##y)){\
        EXECUTE(DMA_HALF_CB(x,y));\
        DMA_ClearFlag(DMA##x##_IT_HT##y);\
    }\
}\


#define DMA_XY_BIND_DONE_TEMPLATE(x,y)\
case y:\
DMA_DONE_CB(x,y) = cb;\
break;\

#define DMA_XY_BIND_HALF_TEMPLATE(x,y)\
case y:\
DMA_HALF_CB(x,y) = cb;\
break;\


#define DMA1_BIND_CB_TEMPLATE(uname)\
case 1:\
    switch(channel_index){\
                DMA_XY_BIND_##uname##_TEMPLATE(1, 1)\
                DMA_XY_BIND_##uname##_TEMPLATE(1, 2)\
                DMA_XY_BIND_##uname##_TEMPLATE(1, 3)\
                DMA_XY_BIND_##uname##_TEMPLATE(1, 4)\
                DMA_XY_BIND_##uname##_TEMPLATE(1, 5)\
                DMA_XY_BIND_##uname##_TEMPLATE(1, 6)\
                DMA_XY_BIND_##uname##_TEMPLATE(1, 7)\
    }\
    break;\

#define DMA2_BIND_CB_TEMPLATE(uname)\
case 2:\
    switch(channel_index){\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 1)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 2)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 3)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 4)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 5)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 6)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 7)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 8)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 9)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 10)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 11)\
    }\
    break;\


#ifdef HAVE_DMA2

#define DMA_BIND_CB_TEMPLATE(name,uname)\
void DmaChannel::bind##name##Cb(Callback && cb){\
    switch(dma_index){\
        DMA1_BIND_CB_TEMPLATE(uname)
        DMA2_BIND_CB_TEMPLATE(uname)
    }\
}\

#else

#define DMA_BIND_CB_TEMPLATE(name,uname)\
void DmaChannel::bind##name##Cb(Callback && cb){\
    switch(dma_index){\
        DMA1_BIND_CB_TEMPLATE(uname)\
    }\
}\

#endif

#ifdef HAVE_DMA1
DMA_IT_TEMPLATE(1,1);
DMA_IT_TEMPLATE(1,2);
DMA_IT_TEMPLATE(1,3);
DMA_IT_TEMPLATE(1,4);
DMA_IT_TEMPLATE(1,5);
DMA_IT_TEMPLATE(1,6);
DMA_IT_TEMPLATE(1,7);
#endif

#ifdef HAVE_DMA2
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

DMA_BIND_CB_TEMPLATE(Done, DONE)
DMA_BIND_CB_TEMPLATE(Half, HALF)

#undef DMA_DONE_CB
#undef DMA_IT_TEMPLATE
#undef DMA_XY_BIND_DONE_TEMPLATE
#undef DMA_XY_BIND_HALF_TEMPLATE
#undef DMA1_BIND_CB_TEMPLATE
#undef DMA2_BIND_CB_TEMPLATE
#undef DMA_BIND_CB_TEMPLATE

#ifdef HAVE_DMA1
DmaChannel dma1Ch1{DMA1_Channel1};
DmaChannel dma1Ch2{DMA1_Channel2};
DmaChannel dma1Ch3{DMA1_Channel3};
DmaChannel dma1Ch4{DMA1_Channel4};
DmaChannel dma1Ch5{DMA1_Channel5};
DmaChannel dma1Ch6{DMA1_Channel6};
DmaChannel dma1Ch7{DMA1_Channel7};
#endif

#ifdef HAVE_DMA2
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

void DmaChannel::enableRcc(){

    #ifdef HAVE_DMA2
    if(instance < DMA2_Channel1){
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    }else{
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
    }

    #else
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    #endif
}

uint8_t DmaChannel::getChannelIndex(const DMA_Channel_TypeDef * _instance){
    uint8_t _dma_index = getDmaIndex(_instance);
    switch(_dma_index){
        case 1:
            return ((uint32_t)_instance - DMA1_Channel1_BASE) / (DMA1_Channel2_BASE - DMA1_Channel1_BASE) + 1;

        #ifdef HAVE_DMA2
        case 2:
            if((uint32_t)_instance < DMA2_Channel7_BASE){ 
                return ((uint32_t)_instance - DMA2_Channel1_BASE) / (DMA2_Channel2_BASE - DMA2_Channel1_BASE) + 1;
            }else{
                return ((uint32_t)_instance - DMA2_Channel7_BASE) / (DMA2_Channel8_BASE - DMA2_Channel7_BASE) + 7;
            }
        #endif
        default:
            return 1;
    }
}

uint32_t DmaChannel::getDoneFlag(){
    uint32_t flag = 0;
    
    switch(dma_index){
        case 1:
            flag = DMA1_IT_TC1 << ((CTZ(DMA1_IT_TC2) - CTZ(DMA1_IT_TC1)) * (channel_index - 1));
            break;
        #ifdef HAVE_DMA2
        case 2:
            flag = DMA2_IT_TC1 << ((CTZ(DMA2_IT_TC2) - CTZ(DMA2_IT_TC1)) * (channel_index - 1));
            break;
        #endif
        default:
            break;
    }
    return flag;
}

uint32_t DmaChannel::getHalfFlag(){
    uint32_t flag = 0;
    
    switch(dma_index){
        case 1:
            flag = DMA1_IT_HT1 << ((CTZ(DMA1_IT_HT2) - CTZ(DMA1_IT_HT1)) * (channel_index - 1));
            break;
        #ifdef HAVE_DMA2
        case 2:
            flag = DMA2_IT_HT1 << ((CTZ(DMA2_IT_HT2) - CTZ(DMA2_IT_HT1)) * (channel_index - 1));
            break;
        #endif
        default:
            break;
    }
    return flag;
}

void DmaChannel::init(const Mode _mode,const Priority priority){
    enableRcc();
    mode = _mode;
    DMA_InitTypeDef DMA_InitStructure = {0};
    DMA_DeInit(instance);

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
            DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)NULL;
            DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)NULL;
            DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
            DMA_InitStructure.DMA_BufferSize = 0;
            DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
            DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
            DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;
            break;
        case Mode::distributeCircular:
            DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
        case Mode::distribute:
            DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)NULL;
            DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)NULL;
            DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
            DMA_InitStructure.DMA_BufferSize = 0;
            DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
            DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
            DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;
            break;
        case Mode::automatic:
            break;
    }

    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;

    DMA_InitStructure.DMA_Priority = (uint32_t)priority;

    DMA_Init(instance, &DMA_InitStructure);
}