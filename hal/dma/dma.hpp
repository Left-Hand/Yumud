#pragma once

#include <stddef.h>
#include <initializer_list>
#include <functional>
#include <type_traits>
#include <array>

#include "core/platform.hpp"

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

enum class DmaIT:uint8_t{
    Done,
    Half
};

struct DmaChannel final{
public:
    using Callback = std::function<void(void)>;
    using Mode = DmaMode;
    using Priority = DmaPriority;

public:

    DmaChannel() = delete;

    DmaChannel(const DmaChannel & other) = delete;
    DmaChannel(DmaChannel && other) = delete;

    DmaChannel(DMA_Channel_TypeDef * inst):
        inst_(inst), 
        done_mask_(calculate_done_mask(inst)),
        half_mask_(calculate_half_mask(inst)),
        dma_index_(calculate_dma_index(inst)),
        channel_index_(calculate_channel_index(inst)){;}

    struct Config{
        const Mode mode;
        const Priority priority;
    };

    void init(const Config & cfg);

    void resume();

    template <typename T>
    void start_transfer_pph2mem(void * dst, const volatile void * src, size_t size){
        set_dst_bits(sizeof(T) << 3);
        set_src_bits(sizeof(T) << 3);

        start_transfer(
            reinterpret_cast<size_t>(dst),
            reinterpret_cast<size_t>(src),
            size
        );
    }

    template <typename T>
    void start_transfer_mem2pph(volatile void * dst, const void * src, size_t size){
        set_dst_bits(sizeof(T) << 3);
        set_src_bits(sizeof(T) << 3);

        start_transfer(
            reinterpret_cast<size_t>(dst),
            reinterpret_cast<size_t>(src),
            size
        );
    }

    template<typename T>
    void start_transfer_mem2mem(void * dst, const void * src, size_t size){
        set_dst_bits(sizeof(T) << 3);
        set_src_bits(sizeof(T) << 3);

        start_transfer(
            reinterpret_cast<size_t>(dst),
            reinterpret_cast<size_t>(src),
            size
        );
    }

    size_t pending();

    void register_nvic(const NvicPriority _priority, const Enable en);

    template<DmaIT I>
    void enable_interrupt(const Enable en){
        if constexpr(I == DmaIT::Half){
            enable_half_it(en);
        }else if constexpr(I == DmaIT::Done){
            enable_done_it(en);
        }
    }

    template<DmaIT I, typename Fn>
    void set_interrupt_callback(Fn && cb){
        if constexpr(I == DmaIT::Half){
            half_cb_ = std::move(cb);
        }else if constexpr(I == DmaIT::Done){
            done_cb_ = std::move(cb);
        }
    }

    bool is_done(){
        return DMA_GetFlagStatus(done_mask_);
    }

private:
    void * inst_;
    
    const uint32_t done_mask_;
    const uint32_t half_mask_;
    
    const uint8_t dma_index_;
    const uint8_t channel_index_;
    
    Callback done_cb_;
    Callback half_cb_;
    Mode mode_;


    void enable_done_it(const Enable en);
    void enable_half_it(const Enable en);

    void enable_rcc(const Enable en);

    void set_periph_width(const size_t width);

    void set_mem_width(const size_t width);

    __fast_inline void set_dst_bits(const size_t width){
        if(dst_is_periph(mode_)){
            set_periph_width(width);
        }else{
            set_mem_width(width);
        }
    }

    __fast_inline void set_src_bits(const size_t width){
        if(!dst_is_periph(mode_)){
            set_periph_width(width);
        }else{
            set_mem_width(width);
        }
    }

    static constexpr uint8_t calculate_dma_index(const void * inst){
        #ifdef ENABLE_DMA2
        return inst < DMA2_Channel1 ? 1 : 2;
        #else
        return 1;
        #endif
    }

    static constexpr uint8_t calculate_channel_index(const void * inst){
        uint8_t dma_index = calculate_dma_index(inst);
        switch(dma_index){
            #ifdef ENABLE_DMA1
            case 1:
                return (reinterpret_cast<uint32_t>(inst) - DMA1_Channel1_BASE) / 
                    (DMA1_Channel2_BASE - DMA1_Channel1_BASE) + 1;
            #endif

            #ifdef ENABLE_DMA2
            case 2:
                if(reinterpret_cast<uint32_t>(inst) < DMA2_Channel7_BASE){ 
                    return ((reinterpret_cast<uint32_t>(inst) - DMA2_Channel1_BASE) / 
                        (DMA2_Channel2_BASE - DMA2_Channel1_BASE)) + 1;
                }else{
                    return ((reinterpret_cast<uint32_t>(inst) - DMA2_Channel7_BASE) / 
                        (DMA2_Channel8_BASE - DMA2_Channel7_BASE)) + 7;
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

    static constexpr uint32_t calculate_done_mask(const void * inst){
        uint8_t dma_index = calculate_dma_index(inst);
        uint8_t channel_index = calculate_channel_index(inst);
        switch(dma_index){
            #ifdef ENABLE_DMA1
            case 1:
                return (DMA1_IT_TC1 << ((CTZ(DMA1_IT_TC2) - CTZ(DMA1_IT_TC1)) * (channel_index - 1)));
            #endif
            #ifdef ENABLE_DMA2
            case 2:
                if(reinterpret_cast<uint32_t>(inst) <= DMA2_Channel7_BASE){ 
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


    static constexpr uint32_t calculate_half_mask(const void * inst){
        uint8_t dma_index = calculate_dma_index(inst);
        uint8_t channel_index = calculate_channel_index(inst);
        switch(dma_index){
            #ifdef ENABLE_DMA1
            case 1:
                return (DMA1_IT_HT1 << ((CTZ(DMA1_IT_HT2) - CTZ(DMA1_IT_HT1)) * (channel_index - 1)));
            #endif
            #ifdef ENABLE_DMA2
            case 2:
                if(reinterpret_cast<uint32_t>(inst) <= DMA2_Channel7_BASE){ 
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

    __fast_inline void on_transfer_half_interrupt(){
        EXECUTE(half_cb_);
    }
    
    __fast_inline void on_transfer_done_interrupt(){
        EXECUTE(done_cb_);
    }

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



    void start_transfer(size_t dst, const size_t src, size_t size);
    

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
