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

enum class DmaEvent:uint8_t{
    TransferComplete,
    HalfTransfer,
    TransferError
};

struct DmaChannel final{
public:
    using Callback = std::function<void(DmaEvent)>;
    using Mode = DmaMode;
    using Priority = DmaPriority;

    explicit DmaChannel(DMA_Channel_TypeDef * inst):
        inst_(inst), 
        done_mask_(dma_details::calculate_done_mask(inst)),
        half_mask_(dma_details::calculate_half_mask(inst)),
        dma_index_(dma_details::calculate_dma_index(inst)),
        channel_index_(dma_details::calculate_channel_index(inst)){;}

    DmaChannel() = delete;

    DmaChannel(const DmaChannel & other) = delete;
    DmaChannel(DmaChannel && other) = delete;



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

    [[nodiscard]] size_t remaining();

    void register_nvic(const NvicPriority _priority, const Enable en);

    template<DmaIT I>
    void enable_interrupt(const Enable en){
        if constexpr(I == DmaIT::Half){
            enable_half_it(en);
        }else if constexpr(I == DmaIT::Done){
            enable_done_it(en);
        }
    }

    template<typename Fn>
    void set_event_callback(Fn && cb){
        callback_ = std::forward<Fn>(cb);
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
    
    Callback callback_;
    Mode mode_ = Mode::ToMemory;


    void enable_done_it(const Enable en);
    void enable_half_it(const Enable en);

    void enable_rcc(const Enable en);

    void set_periph_width(const size_t width);

    void set_mem_width(const size_t width);

    __fast_inline void set_dst_bits(const size_t width){
        if(mode_.dst_is_periph()){
            set_periph_width(width);
        }else{
            set_mem_width(width);
        }
    }

    __fast_inline void set_src_bits(const size_t width){
        if(not mode_.dst_is_periph()){
            set_periph_width(width);
        }else{
            set_mem_width(width);
        }
    }



    
    __fast_inline void on_interrupt(DmaEvent event){
        EXECUTE(callback_, event);
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
