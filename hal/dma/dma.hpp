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
        set_src_and_dst_bytes(sizeof(T), sizeof(T));

        start_transfer(
            reinterpret_cast<size_t>(dst),
            reinterpret_cast<size_t>(src),
            size
        );
    }

    template <typename T>
    void start_transfer_mem2pph(volatile void * dst, const void * src, size_t size){
        set_src_and_dst_bytes(sizeof(T), sizeof(T));

        start_transfer(
            reinterpret_cast<size_t>(dst),
            reinterpret_cast<size_t>(src),
            size
        );
    }

    template<typename T>
    void start_transfer_mem2mem(void * dst, const void * src, size_t size){
        set_src_and_dst_bytes(sizeof(T), sizeof(T));

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
    void set_event_handler(Fn && cb){
        callback_ = std::forward<Fn>(cb);
    }

    [[nodiscard]] bool is_done(){
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

    __fast_inline void set_src_and_dst_bytes(
        const size_t src_bytes, 
        const size_t dst_bytes
    ){ 
        if(mode_.dst_is_periph()){
            set_mem_and_periph_bytes(src_bytes, dst_bytes);
        }else{
            set_mem_and_periph_bytes(dst_bytes, src_bytes);
        }
    }

    void set_mem_and_periph_bytes(
        const size_t src_bytes, 
        const size_t dst_bytes
    );

    __fast_inline void accept_interrupt(DmaEvent event){
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
    extern DmaChannel dma1_ch1;
    extern DmaChannel dma1_ch2;
    extern DmaChannel dma1_ch3;
    extern DmaChannel dma1_ch4;
    extern DmaChannel dma1_ch5;
    extern DmaChannel dma1_ch6;
    extern DmaChannel dma1_ch7;
#endif

#ifdef ENABLE_DMA2
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
