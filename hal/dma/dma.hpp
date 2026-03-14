#pragma once

#include "dma_lld.hpp"
#include "ral/ch32/ch32_common_dma_def.hpp"
#include "core/intrinsics/volatile.hpp"

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


struct DmaIrqHandler{};
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

    void clear_pending_flag_and_restart();

    template <DmaWordSize DST_WS, DmaWordSize SRC_WS>
    void start_transfer_pph2mem(void * dst, const volatile void * src, size_t size){
        set_src_and_dst_wordsize(
            DST_WS,
            SRC_WS
        );

        start_transfer(
            reinterpret_cast<uintptr_t>(dst),
            reinterpret_cast<uintptr_t>(src),
            size
        );
    }

    template <DmaWordSize DST_WS, DmaWordSize SRC_WS>
    void start_transfer_mem2pph(volatile void * dst, const void * src, size_t size){
        set_src_and_dst_wordsize(
            DST_WS,
            SRC_WS
        );

        start_transfer(
            reinterpret_cast<uintptr_t>(dst),
            reinterpret_cast<uintptr_t>(src),
            size
        );
    }

    template <DmaWordSize DST_WS, DmaWordSize SRC_WS>
    void start_transfer_mem2mem(void * dst, const void * src, size_t size){
        set_src_and_dst_wordsize(
            DST_WS,
            SRC_WS
        );

        start_transfer(
            reinterpret_cast<uintptr_t>(dst),
            reinterpret_cast<uintptr_t>(src),
            size
        );
    }

    //返回待传输的数目
    [[nodiscard]] size_t pending_count();

    void register_nvic(const NvicPriorityCode priority_code, const Enable en);

    template<DmaIT I>
    void enable_interrupt(const Enable en){
        if constexpr(I == DmaIT::Half){
            enable_transfer_onhalf_interrupt(en);
        }else if constexpr(I == DmaIT::Done){
            enable_transfer_complete_interrupt(en);
        }
    }

    template<typename Fn>
    void set_event_callback(Fn && cb){
        event_callback_ = std::forward<Fn>(cb);
    }

    [[nodiscard]] bool is_transfer_complete();

public:
    void * inst_;
    
    const Nth dma_nth_;
    const Nth ch_sel_nth_;

    const uint32_t transfer_complete_mask_;
    const uint32_t transfer_onhalf_mask_;
    
    Mode mode_ = Mode::Default;
    
    Callback event_callback_ = nullptr;


    void enable_transfer_complete_interrupt(const Enable en);
    void enable_transfer_onhalf_interrupt(const Enable en);

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

    __fast_inline
    void set_mem_and_periph_wordsize(
        const WordSize mem_wordsize, 
        const WordSize periph_wordsize
    ){ 
        auto * dma_ch = reinterpret_cast<ral::DMA_CH_Def *>(inst_);
        intrinsics::modify_reg(&dma_ch->CFGR, [&](auto reg){
            reg.MSIZE = static_cast<uint8_t>(mem_wordsize);
            reg.PSIZE = static_cast<uint8_t>(periph_wordsize);
            return reg;
        });
    }

    __fast_inline void on_interrupt(DmaEvent event){
        EXECUTE(event_callback_, event);
    }

    
    void start_transfer(const uintptr_t dst_addr, const uintptr_t src_addr, size_t size);

    friend class DmaIrqHandler;
};


}
