#pragma once

#include <stddef.h>
#include <initializer_list>
#include <functional>
#include <type_traits>
#include <array>

#include "core/platform.hpp"
#include "core/sdk.hpp"

#include "hal/nvic/nvic.hpp"

#include "dma_utils.hpp"



extern"C"{
#ifdef ENABLE_DMA1
    __interrupt void DMA1_Channel1_IRQHandler(void);
    __interrupt void DMA1_Channel2_IRQHandler(void);
    __interrupt void DMA1_Channel3_IRQHandler(void);
    __interrupt void DMA1_Channel4_IRQHandler(void);
    __interrupt void DMA1_Channel5_IRQHandler(void);
    __interrupt void DMA1_Channel6_IRQHandler(void);
    __interrupt void DMA1_Channel7_IRQHandler(void);
#endif

#ifdef ENABLE_DMA2
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

class DmaChannel{
public:
    using Callback = std::function<void(void)>;
    using Mode = DmaMode;
    using Priority = DmaPriority;
protected:
    DMA_Channel_TypeDef * instance;
    
    const uint32_t done_mask;
    const uint32_t half_mask;
    
    const uint8_t dma_index;
    const uint8_t channel_index;
    
    Callback done_cb_;
    Callback half_cb_;
    Mode mode_;

    void enable_rcc(const bool en);
    void config_periph_data_bytes(const size_t bytes){
        uint32_t tmpreg = instance->CFGR;
        tmpreg &= ((~(0b11u << 8)));
        tmpreg |= (bytes - 1) << 8;
        instance->CFGR = tmpreg;
    }

    void config_mem_data_bytes(const size_t bytes){
        uint32_t tmpreg = instance->CFGR;
        tmpreg &= ((~(0b11u << 10)));
        tmpreg |= (bytes - 1) << 10;
        instance->CFGR = tmpreg;
    }

    void config_dst_mem_data_bytes(const size_t bytes){
        if(dst_is_periph(mode_)){
            config_periph_data_bytes(bytes);
        }else{
            config_mem_data_bytes(bytes);
        }
    }

    void config_src_mem_data_bytes(const size_t bytes){
        if(!dst_is_periph(mode_)){
            config_periph_data_bytes(bytes);
        }else{
            config_mem_data_bytes(bytes);
        }
    }

    static constexpr uint8_t calculate_dma_index(const DMA_Channel_TypeDef * _instance){
        #ifdef ENABLE_DMA2
        return _instance < DMA2_Channel1 ? 1 : 2;
        #else
        return 1;
        #endif
    }

    static constexpr uint8_t calculate_channel_index(const DMA_Channel_TypeDef * _instance){
        uint8_t dma_index = calculate_dma_index(_instance);
        switch(dma_index){
            #ifdef ENABLE_DMA1
            case 1:
                return (uint32_t(_instance) - DMA1_Channel1_BASE) / (DMA1_Channel2_BASE - DMA1_Channel1_BASE) + 1;
            #endif

            #ifdef ENABLE_DMA2
            case 2:
                if(uint32_t(_instance) < DMA2_Channel7_BASE){ 
                    return (uint32_t(_instance) - DMA2_Channel1_BASE) / (DMA2_Channel2_BASE - DMA2_Channel1_BASE) + 1;
                }else{
                    return (uint32_t(_instance) - DMA2_Channel7_BASE) / (DMA2_Channel8_BASE - DMA2_Channel7_BASE) + 7;
                }
            #endif
            default:
                return 1;
        }
    }

    static constexpr bool dst_is_periph(const Mode mode){
        switch(mode){
            case Mode::toPeriph:
            case Mode::toPeriphCircular:
                return true;
            default:
                return false;
        }
    }

    static constexpr uint32_t calculate_done_mask(const DMA_Channel_TypeDef * _instance){
        uint8_t dma_index = calculate_dma_index(_instance);
        uint8_t channel_index = calculate_channel_index(_instance);
        switch(dma_index){
            #ifdef ENABLE_DMA1
            case 1:
                return (DMA1_IT_TC1 << ((CTZ(DMA1_IT_TC2) - CTZ(DMA1_IT_TC1)) * (channel_index - 1)));
            #endif
            #ifdef ENABLE_DMA2
            case 2:
                if(uint32_t(_instance) <= DMA2_Channel7_BASE){ 
                    return ((uint32_t)(DMA2_IT_TC1 & 0xff) << ((CTZ(DMA2_IT_TC2) - CTZ(DMA2_IT_TC1)) * (channel_index - 1))) | (uint32_t)(0x10000000);
                }else{
                    return ((uint32_t)(DMA2_IT_TC8 & 0xff) << ((CTZ(DMA2_IT_TC9) - CTZ(DMA2_IT_TC8)) * (channel_index - 8))) | (uint32_t)(0x20000000);
                }
            #endif
            default:
                break;
        }
        return 0;
    }


    static constexpr uint32_t calculate_half_mask(const DMA_Channel_TypeDef * _instance){
        uint8_t dma_index = calculate_dma_index(_instance);
        uint8_t channel_index = calculate_channel_index(_instance);
        switch(dma_index){
            #ifdef ENABLE_DMA1
            case 1:
                return (DMA1_IT_HT1 << ((CTZ(DMA1_IT_HT2) - CTZ(DMA1_IT_HT1)) * (channel_index - 1)));
            #endif
            #ifdef ENABLE_DMA2
            case 2:
                if(uint32_t(_instance) <= DMA2_Channel7_BASE){ 
                    return ((uint32_t)(DMA2_IT_HT1 & 0xff) << ((CTZ(DMA2_IT_HT2) - CTZ(DMA2_IT_HT1)) * (channel_index - 1))) | (uint32_t)(0x10000000);
                }else{
                    return ((uint32_t)(DMA2_IT_HT8 & 0xff) << ((CTZ(DMA2_IT_HT9) - CTZ(DMA2_IT_HT8)) * (channel_index - 8))) | (uint32_t)(0x20000000);
                }
            #endif
            default:
                break;
        }
        return 0;
    }

    void on_transfer_half_interrupt();
    void on_transfer_done_interrupt();

    #ifdef ENABLE_DMA1
        friend void ::DMA1_Channel1_IRQHandler(void);
        friend void ::DMA1_Channel2_IRQHandler(void);
        friend void ::DMA1_Channel3_IRQHandler(void);
        friend void ::DMA1_Channel4_IRQHandler(void);
        friend void ::DMA1_Channel5_IRQHandler(void);
        friend void ::DMA1_Channel6_IRQHandler(void);
        friend void ::DMA1_Channel7_IRQHandler(void);
    #endif

    #ifdef ENABLE_DMA2
        friend void ::DMA2_Channel1_IRQHandler(void);
        friend void ::DMA2_Channel2_IRQHandler(void);
        friend void ::DMA2_Channel3_IRQHandler(void);
        friend void ::DMA2_Channel4_IRQHandler(void);
        friend void ::DMA2_Channel5_IRQHandler(void);
        friend void ::DMA2_Channel6_IRQHandler(void);
        friend void ::DMA2_Channel7_IRQHandler(void);
        friend void ::DMA2_Channel8_IRQHandler(void);
        friend void ::DMA2_Channel9_IRQHandler(void);
        friend void ::DMA2_Channel10_IRQHandler(void);
        friend void ::DMA2_Channel11_IRQHandler(void);
    #endif



    void start(void * dst, const void * src, size_t size);
    
    // template <typename T>
    // void start(T * dst, const T * src, size_t size){
    //     config_dst_mem_data_bytes(sizeof(T));
    //     config_src_mem_data_bytes(sizeof(T));
    //     start(
    //         reinterpret_cast<void *>(dst), 
    //         reinterpret_cast<const void *>(src), 
    //         size);
    // }
public:

    DmaChannel() = delete;

    DmaChannel(const DmaChannel & other) = delete;
    DmaChannel(DmaChannel && other) = delete;

    DmaChannel(DMA_Channel_TypeDef * _instance):
        instance(_instance), 
        done_mask(calculate_done_mask(instance)),
        half_mask(calculate_half_mask(instance)),
        dma_index(calculate_dma_index(_instance)),
        channel_index(calculate_channel_index(_instance)){;}

    void init(const Mode mode,const Priority priority = Priority::medium);

    void resume(){
        DMA_ClearFlag(done_mask);
        DMA_ClearFlag(half_mask);

        DMA_Cmd(instance, ENABLE);
    }

    template <typename T>
    void transfer_pph2mem(auto * dst, const volatile auto * src, size_t size){
        config_dst_mem_data_bytes(sizeof(T));
        config_src_mem_data_bytes(sizeof(T));

        start(
            reinterpret_cast<void *>(dst), 
            reinterpret_cast<const void *>(const_cast<const T *>(
                reinterpret_cast<const volatile T *>(src))), 
            size
        );
    }

    template <typename T>
    void transfer_mem2pph(volatile auto * dst, const auto * src, size_t size){
        config_dst_mem_data_bytes(sizeof(T));
        config_src_mem_data_bytes(sizeof(T));

        start(
            reinterpret_cast<void *>(const_cast<T *>(reinterpret_cast<volatile T *>(dst))), 
            reinterpret_cast<const void *>(src), 
            size
        );
    }

    template<typename T>
    void transfer_mem2mem(auto * dst, const auto * src, size_t size){
        config_dst_mem_data_bytes(sizeof(T));
        config_src_mem_data_bytes(sizeof(T));

        start(
            reinterpret_cast<void *>(const_cast<T *>((dst))), 
            reinterpret_cast<const void *>(src), 
            size
        );
    }


    // template <typename U, typename T>
    // requires std::is_array_v<T>
    // void start(U * dst, const T & src){//TODO array can only be c-ctyle array
    //     config_dst_mem_data_bytes(sizeof(U));
    //     config_src_mem_data_bytes(sizeof(std::remove_extent_t<T>));
    //     start(reinterpret_cast<void *>(dst), reinterpret_cast<const void *>(&src[0]) , std::distance(std::begin(src), std::end(src)));
    // }

    // template <typename U, typename T>
    // requires std::is_array_v<U>
    // void start(U & dst, const T * src){
    //     config_dst_mem_data_bytes(sizeof(U));
    //     config_src_mem_data_bytes(sizeof(std::remove_extent_t<T>));
    //     start(
    //         reinterpret_cast<void *>(dst), 
    //         reinterpret_cast<const void *>(src), 
    //         std::distance(std::begin(dst), std::end(dst))
    //     );
    // }

    void config_data_bytes(const size_t bytes){
        config_mem_data_bytes(bytes);
        config_periph_data_bytes(bytes);
    }

    size_t pending(){
        return instance -> CNTR;
    }

    void enable_it(const NvicPriority _priority, const bool en = true);

    void enable_done_it(const bool en = true){
        DMA_ClearITPendingBit(done_mask);
        DMA_ITConfig(instance, DMA_IT_TC, en);
    }

    void enable_half_it(const bool en = true){
        DMA_ClearITPendingBit(half_mask);
        DMA_ITConfig(instance, DMA_IT_HT, en);
    }


    void bind_done_cb(auto && cb){
        done_cb_ = std::move(cb);
    }

    void bind_half_cb(auto && cb){
        half_cb_ = std::move(cb);
    }

    bool done(){
        return DMA_GetFlagStatus(done_mask);
    }
};

#ifdef ENABLE_DMA1
    extern DmaChannel dma1Ch1;
    extern DmaChannel dma1Ch2;
    extern DmaChannel dma1Ch3;
    extern DmaChannel dma1Ch4;
    extern DmaChannel dma1Ch5;
    extern DmaChannel dma1Ch6;
    extern DmaChannel dma1Ch7;
#endif

#ifdef ENABLE_DMA2
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
}
