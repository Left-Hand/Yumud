#pragma once

#include "sys/platform.h"

#include "hal/gpio/gpio.hpp"
#include "hal/gpio/port.hpp"
#include "hal/nvic/nvic.hpp"

#include "dma_utils.hpp"

#include <stddef.h>
#include <initializer_list>
#include <functional>
#include <type_traits>
#include <array>


#define HAVE_DMA1
// #define HAVE_DMA2

class DmaChannel{

public:

    using Callback = std::function<void(void)>;
    using Mode = DmaUtils::Mode;
    using Priority = DmaUtils::Priority;
protected:
// public:
    DMA_Channel_TypeDef * instance;
    Mode mode;
    uint8_t dma_index;
    uint8_t channel_index;

    uint32_t done_mask;
    uint32_t half_mask;

    void enableRcc();
    void configPeriphDataBytes(const size_t bytes){
        uint32_t tmpreg = instance->CFGR;
        tmpreg &= ((~(0b11 << 8)));
        tmpreg |= (bytes - 1) << 8;
        instance->CFGR = tmpreg;
    }

    void configMemDataBytes(const size_t bytes){
        uint32_t tmpreg = instance->CFGR;
        tmpreg &= ((~(0b11 << 10)));
        tmpreg |= (bytes - 1) << 10;
        instance->CFGR = tmpreg;
    }

    bool periphIsDst() const{
        if(mode == Mode::toMem || mode == Mode::toMemCircular) return false;
        else return true;
    }

    void configDstMemDataBytes(const size_t bytes){
        if(periphIsDst()){
            configPeriphDataBytes(bytes);
        }else{
            configMemDataBytes(bytes);
        }
    }

    void configSrcMemDataBytes(const size_t bytes){
        if(!periphIsDst()){
            configPeriphDataBytes(bytes);
        }else{
            configMemDataBytes(bytes);
        }
    }

    constexpr uint8_t getDmaIndex(const DMA_Channel_TypeDef * _instance){
        #ifdef HAVE_DMA2
        return _instance < DMA2_Channel1 ? 1 : 2;
        #else
        return 1;
        #endif
    }

    constexpr uint8_t getChannelIndex(const DMA_Channel_TypeDef * _instance){
        switch(dma_index){
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

    constexpr uint32_t getDoneMask(){
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


    constexpr uint32_t getHalfMask(){
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

public:

    DmaChannel() = delete;
    DmaChannel(DMA_Channel_TypeDef * _instance):
                instance(_instance), 
                dma_index(getDmaIndex(_instance)),
                channel_index(getChannelIndex(_instance)),
                done_mask(getDoneMask()),
                half_mask(getHalfMask()){;}

    void init(const Mode _mode,const Priority priority = Priority::medium);

    void begin(){
        DMA_ClearFlag(done_mask);
        DMA_ClearFlag(half_mask);

        DMA_Cmd(instance, ENABLE);
    }

    void begin(void * dst, const void * src, size_t size){

        if(!periphIsDst()){
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
        configDstMemDataBytes(sizeof(T));
        configSrcMemDataBytes(sizeof(T));
        begin((void *)dst, (const void *)src, size);
    }

    template <typename U, typename T>
    requires std::is_array_v<T>
    void begin(U * dst, const T & src){//TODO array can only be c-ctyle array
        configDstMemDataBytes(sizeof(U));
        configSrcMemDataBytes(sizeof(std::remove_extent_t<T>));
        begin((void *)dst, (const void *)&src[0], std::distance(std::begin(src), std::end(src)));
    }

    template <typename U, typename T>
    requires std::is_array_v<U>
    void begin(U & dst, const T * src){
        configDstMemDataBytes(sizeof(U));
        configSrcMemDataBytes(sizeof(std::remove_extent_t<T>));
        begin((void *)&dst[0], (const void *)src, std::distance(std::begin(dst), std::end(dst)));
    }

    void configDataBytes(const size_t bytes){
        configMemDataBytes(bytes);
        configPeriphDataBytes(bytes);
    }

    size_t pending(){
        return instance -> CNTR;
    }

    void enableIt(const NvicPriority _priority, const bool en = true);

    void enableDoneIt(const bool en = true){
        DMA_ClearITPendingBit(done_mask);
        DMA_ITConfig(instance, DMA_IT_TC, en);
    }

    void enableHalfIt(const bool en = true){
        DMA_ClearITPendingBit(half_mask);
        DMA_ITConfig(instance, DMA_IT_HT, en);
    }


    void bindDoneCb(Callback && cb);
    void bindHalfCb(Callback && cb);

    bool isDone(){
        return DMA_GetFlagStatus(done_mask);
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
