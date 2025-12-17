#pragma once

#include <stddef.h>
#include <initializer_list>
#include <functional>
#include <type_traits>
#include <array>

#include "hal/nvic/nvic.hpp"

#include "dma_utils.hpp"



extern"C"{
#ifdef DMA1_PRESENT
    __interrupt void DMA1_Channel1_IRQHandler(void);
    __interrupt void DMA1_Channel2_IRQHandler(void);
    __interrupt void DMA1_Channel3_IRQHandler(void);
    __interrupt void DMA1_Channel4_IRQHandler(void);
    __interrupt void DMA1_Channel5_IRQHandler(void);
    __interrupt void DMA1_Channel6_IRQHandler(void);
    __interrupt void DMA1_Channel7_IRQHandler(void);
#endif

#ifdef DMA2_PRESENT
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

namespace details{
template<typename T>
static constexpr dma::WordSize type_to_dma_wordsize_v = [] -> dma::WordSize{
    switch(sizeof(T)){
        case 1: return dma::WordSize::OneByte;
        case 2: return dma::WordSize::TwoByte;
        case 4: return dma::WordSize::FourByte;
    }
    __builtin_trap();
}();
};
struct DmaChannel final{

public:
    using Callback = std::function<void(DmaEvent)>;
    using Mode = DmaMode;
    using Priority = DmaPriority;
    using WordSize = DmaWordSize;

    explicit DmaChannel(void * inst);
    DmaChannel() = delete;

    DmaChannel(const DmaChannel & other) = delete;
    DmaChannel(DmaChannel && other) = delete;



    struct Config{
        const Mode mode;
        const Priority priority;
    };

    void init(const Config & cfg);

    void clear_and_start();

    template <typename T>
    void start_transfer_pph2mem(void * dst, const volatile void * src, size_t size){
        set_src_and_dst_wordsize(
            details::type_to_dma_wordsize_v<T>, 
            details::type_to_dma_wordsize_v<T>
        );

        start_transfer(
            reinterpret_cast<size_t>(dst),
            reinterpret_cast<size_t>(src),
            size
        );
    }

    template <typename T>
    void start_transfer_mem2pph(volatile void * dst, const void * src, size_t size){
        set_src_and_dst_wordsize(
            details::type_to_dma_wordsize_v<T>, 
            details::type_to_dma_wordsize_v<T>
        );

        start_transfer(
            reinterpret_cast<size_t>(dst),
            reinterpret_cast<size_t>(src),
            size
        );
    }

    template<typename T>
    void start_transfer_mem2mem(void * dst, const void * src, size_t size){
        set_src_and_dst_wordsize(
            details::type_to_dma_wordsize_v<T>, 
            details::type_to_dma_wordsize_v<T>
        );

        start_transfer(
            reinterpret_cast<size_t>(dst),
            reinterpret_cast<size_t>(src),
            size
        );
    }

    //返回待传输的数目
    [[nodiscard]] size_t pending_count();

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

    [[nodiscard]] bool is_done();

private:
    void * inst_;
    
    const uint32_t done_mask_;
    const uint32_t half_mask_;
    
    const uint8_t dma_index_;
    const uint8_t channel_index_;
    
    Callback callback_;
    Mode mode_ = Mode::Default;


    void enable_done_it(const Enable en);
    void enable_half_it(const Enable en);

    void enable_rcc(const Enable en);

    __fast_inline void set_src_and_dst_wordsize(
        const WordSize src_wordsize, 
        const WordSize dst_wordsize
    ){ 
        if(mode_.dst_is_periph()){
            set_mem_and_periph_wordsize(src_wordsize, dst_wordsize);
        }else{
            set_mem_and_periph_wordsize(dst_wordsize, src_wordsize);
        }
    }

    void set_mem_and_periph_wordsize(
        const WordSize src_wordsize, 
        const WordSize dst_wordsize
    );

    __fast_inline void accept_interrupt(DmaEvent event){
        EXECUTE(callback_, event);
    }
    

    #ifdef DMA1_PRESENT
        friend void ::DMA1_Channel1_IRQHandler(void);
        friend void ::DMA1_Channel2_IRQHandler(void);
        friend void ::DMA1_Channel3_IRQHandler(void);
        friend void ::DMA1_Channel4_IRQHandler(void);
        friend void ::DMA1_Channel5_IRQHandler(void);
        friend void ::DMA1_Channel6_IRQHandler(void);
        friend void ::DMA1_Channel7_IRQHandler(void);
    #endif

    #ifdef DMA2_PRESENT
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

    void start_transfer(const size_t dst_addr, const size_t src_addr, size_t size);
};

#ifdef DMA1_PRESENT
    extern DmaChannel dma1_ch1;
    extern DmaChannel dma1_ch2;
    extern DmaChannel dma1_ch3;
    extern DmaChannel dma1_ch4;
    extern DmaChannel dma1_ch5;
    extern DmaChannel dma1_ch6;
    extern DmaChannel dma1_ch7;
#endif

#ifdef DMA2_PRESENT
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
