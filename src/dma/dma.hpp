#ifndef __DMA_HPP__

#define __DMA_HPP__

#include "src/platform.h"
#include "src/gpio/gpio.hpp"
#include "src/gpio/port.hpp"
#include "src/nvic/nvic.hpp"
#include <stddef.h>
#include <initializer_list>

#define HAVE_DMA1
// #define HAVE_DMA2

class DmaChannel{

public:
    enum class Mode:uint8_t{
        toMem = DMA_DIR_PeripheralSRC,
        toPeriph = DMA_DIR_PeripheralDST,
        synergy,
        distribute,

        toMemCircular = DMA_DIR_PeripheralSRC | 0x80,
        toPeriphCircular = DMA_DIR_PeripheralDST | 0x80,
        synergyCircular,
        distributeCircular,
        automatic
    };

    enum class Priority:uint16_t{
        low = DMA_Priority_Low,
        medium = DMA_Priority_Medium,
        high = DMA_Priority_High,
        ultra = DMA_Priority_VeryHigh
    };

// protected:
public:
    DMA_Channel_TypeDef * instance;
    uint8_t dma_index;
    uint8_t channel_index;

    constexpr bool isSFR(void * ptr){
        return ((uint32_t)ptr > 0x40000000);
    }

    constexpr bool isRam(void * ptr){
        return  (not isSFR(ptr))and ((uint32_t)ptr > 0x20000000);
    }

    constexpr bool isRom(void * ptr){
        return ((uint32_t)ptr < 0x20000000);
    }

    constexpr bool isAlign(void * ptr){
        return ((uint32_t)ptr & 0x3) == 0;
    }

    void enableRcc(){

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
    void configPeriphDataBits(const uint8_t bits){
        uint32_t tmpreg = instance->CFGR;
        tmpreg &= ((~(0b11 << 8)));
        tmpreg |= (bits/8 - 1) << 8;
        instance->CFGR = tmpreg;

    }

    void configMemDataBits(const uint8_t bits){
        uint32_t tmpreg = instance->CFGR;
        tmpreg &= ((~(0b11 << 10)));
        tmpreg |= (bits/8 - 1) << 10;
        instance->CFGR = tmpreg;
    }

    uint8_t getDmaIndex(const DMA_Channel_TypeDef * _instance){
        #ifdef HAVE_DMA2
        return _instance < DMA2_Channel1 ? 1 : 2;
        #else
        return 1;
        #endif
    }
    uint8_t getChannelIndex(const DMA_Channel_TypeDef * _instance){
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

    uint32_t getDoneFlag(){
        uint32_t flag = 0;
        
        switch(dma_index){
            case 1:
                flag = DMA1_IT_TC1 + (DMA1_IT_TC2 - DMA1_IT_TC1) * channel_index;
                break;
            #ifdef HAVE_DMA2
            case 2:
                flag = DMA2_IT_TC1 + (DMA2_IT_TC2 - DMA2_IT_TC1) * channel_index;
                break;
            #endif
            default:
                break;
        }
        return flag;
    }

    uint32_t getHalfFlag(){
        uint32_t flag = 0;
        
        switch(dma_index){
            case 1:
                flag = DMA1_IT_HT1 + (DMA1_IT_HT2 - DMA1_IT_HT1) * channel_index;
                break;
            #ifdef HAVE_DMA2
            case 2:
                flag = DMA2_IT_HT1 + (DMA2_IT_HT2 - DMA2_IT_HT1) * channel_index;
                break;
            #endif
            default:
                break;
        }
        return flag;
    }
public:
    DmaChannel(DMA_Channel_TypeDef * _instance):
                instance(_instance), 
                dma_index(getDmaIndex(_instance)),
                channel_index(getChannelIndex(_instance)){;}

    void init(const Mode mode,const Priority priority = Priority::medium){
        enableRcc();

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

    void begin(){
        DMA_ClearFlag(DMA1_IT_TC5);
        DMA_ClearFlag(DMA1_IT_HT5);
        
        DMA_Cmd(instance, ENABLE);
    }

    void begin(void * dst, const void * src, size_t size){
        instance -> PADDR = (uint32_t)dst;
        instance -> MADDR = (uint32_t)src;
        instance -> CNTR = size;
        begin();
    }

    template <typename T>
    void begin(T * dst, const T * src, size_t size){
        configMemDataBits(sizeof(T) * 8);
        configPeriphDataBits(sizeof(T) * 8);
        begin((void *)dst, (const void *)src, size);
    }

    size_t pending(){
        return instance -> CNTR;
    }

    void enableIt(NvicPriority _priority){

        int irq = int(IRQn_Type::Software_IRQn);
        switch(dma_index){
            case 1:
                irq = DMA1_Channel1_IRQn + (DMA1_Channel2_IRQn - DMA1_Channel1_IRQn) * (dma_index - 1);
                break;
            #ifdef HAVE_DMA2
            case 2:
                if(channel_index <= 5){
                    irq = DMA2_Channel1_IRQn + (DMA2_Channel2_IRQn - DMA2_Channel1_IRQn) * (channel_index - 1);
                }else{
                    irq = DMA2_Channel6_IRQn + (DMA2_Channel7_IRQn - DMA2_Channel6_IRQn) * (channel_index - 6);
                }
                break;
            #endif
        }

        _priority.enable(IRQn(irq), ENABLE);
    }

    void enableDoneIt(const bool en = true){
        DMA_ClearITPendingBit(getDoneFlag());
        DMA_ITConfig(instance, DMA_IT_TC, en);
    }

    void enableHalfIt(const bool en = true){
        DMA_ClearITPendingBit(getHalfFlag());
        DMA_ITConfig(instance, DMA_IT_HT, en);
    }




    bool isDone(){
        return DMA_GetFlagStatus(getDoneFlag());
    }
};


#ifdef HAVE_DMA1
    extern DmaChannel dma1Ch1;
    extern DmaChannel dma1Ch2;
    extern DmaChannel dma1Ch3;
    extern DmaChannel dma1Ch4;
    extern DmaChannel dma1Ch5;
    extern DmaChannel dma1Ch6;
    extern DmaChannel dma1Ch7;
#endif

#ifdef HAVE_DMA2
    extern DmaChannel dma2Ch1;
    extern DmaChannel dma2Ch2;
    extern DmaChannel dma2Ch3;
    extern DmaChannel dma2Ch4;
    extern DmaChannel dma2Ch5;
    extern DmaChannel dma2Ch6;
    extern DmaChannel dma2Ch7;
    extern DmaChannel dma2Ch8;
    extern DmaChannel dma2Ch9;
    extern DmaChannel dma2Ch10;
    extern DmaChannel dma2Ch11;
#endif


#endif