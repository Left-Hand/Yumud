#ifndef __DMA_HPP__

#define __DMA_HPP__

#include "sys/platform.h"
#include "hal/gpio/gpio.hpp"
#include "hal/gpio/port.hpp"
#include "hal/nvic/nvic.hpp"

#include <stddef.h>
#include <initializer_list>
#include <functional>


#define HAVE_DMA1
// #define HAVE_DMA2

class DmaChannel{

public:

    using Callback = std::function<void(void)>;
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
    Mode mode;
    uint8_t dma_index;
    uint8_t channel_index;

    void enableRcc();
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

    uint8_t getChannelIndex(const DMA_Channel_TypeDef * _instance);

    uint32_t getDoneFlag();

    uint32_t getHalfFlag();
public:

    DmaChannel() = delete;
    DmaChannel(DMA_Channel_TypeDef * _instance):
                instance(_instance), 
                dma_index(getDmaIndex(_instance)),
                channel_index(getChannelIndex(_instance)){;}

    void init(const Mode _mode,const Priority priority = Priority::medium);

    void begin(){
        DMA_ClearFlag(DMA1_IT_TC5);
        DMA_ClearFlag(DMA1_IT_HT5);

        DMA_Cmd(instance, ENABLE);
    }

    void begin(void * dst, const void * src, size_t size){
        if(mode == Mode::toMem || mode == Mode::toMemCircular){
            instance -> PADDR = (uint32_t)src;
            instance -> MADDR = (uint32_t)dst;
        }else{
            instance -> PADDR = (uint32_t)dst;
            instance -> MADDR = (uint32_t)src;
        }

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
                irq = (DMA1_Channel1_IRQn + (DMA1_Channel2_IRQn - DMA1_Channel1_IRQn) * (channel_index - 1));
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
        _priority.enable(IRQn(DMA1_Channel4_IRQn), ENABLE);
    }

    void enableDoneIt(const bool en = true){
        DMA_ClearITPendingBit(getDoneFlag());
        DMA_ITConfig(instance, DMA_IT_TC, en);
    }

    void enableHalfIt(const bool en = true){
        DMA_ClearITPendingBit(getHalfFlag());
        DMA_ITConfig(instance, DMA_IT_HT, en);
    }


    void bindDoneCb(Callback && cb);
    void bindHalfCb(Callback && cb);

    bool isDone(){
        return DMA_GetFlagStatus(getDoneFlag());
    }
};

#define DMA_XY_TEMPLATE(x,y)\
extern"C"{__interrupt void DMA##x##_Channel##y##_IRQHandler(void);}\


#ifdef HAVE_DMA1
    extern DmaChannel dma1Ch1;
    extern DmaChannel dma1Ch2;
    extern DmaChannel dma1Ch3;
    extern DmaChannel dma1Ch4;
    extern DmaChannel dma1Ch5;
    extern DmaChannel dma1Ch6;
    extern DmaChannel dma1Ch7;

    DMA_XY_TEMPLATE(1,1)
    DMA_XY_TEMPLATE(1,2)
    DMA_XY_TEMPLATE(1,3)
    DMA_XY_TEMPLATE(1,4)
    DMA_XY_TEMPLATE(1,5)
    DMA_XY_TEMPLATE(1,6)
    DMA_XY_TEMPLATE(1,7)

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

    DMA_XY_TEMPLATE(2,1)
    DMA_XY_TEMPLATE(2,2)
    DMA_XY_TEMPLATE(2,3)
    DMA_XY_TEMPLATE(2,4)
    DMA_XY_TEMPLATE(2,5)
    DMA_XY_TEMPLATE(2,6)
    DMA_XY_TEMPLATE(2,7)
    DMA_XY_TEMPLATE(2,8)
    DMA_XY_TEMPLATE(2,9)
    DMA_XY_TEMPLATE(2,10)
    DMA_XY_TEMPLATE(2,11)
#endif


#endif