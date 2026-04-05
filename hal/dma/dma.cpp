#include "dma.hpp"

#include "core/utils/nth.hpp"
#include "core/sdk.hpp"

using namespace ymd;
using namespace ymd::hal;

#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SPL_INST(x) (reinterpret_cast<COPY_CONST(inst_, DMA_Channel_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::DMA_CH_Def)>(x))


namespace {
static constexpr Nth _dma_calc_nth(const uint32_t inst_base){
    #ifdef DMA2_PRESENT
    return inst_base < DMA2_Channel1_BASE ? Nth(1) : Nth(2);
    #else
    return Nth(1);
    #endif
}

static constexpr Nth _dma_ch_sel_nth(const uint32_t inst_base){
    const Nth dma_nth = _dma_calc_nth(inst_base);
    switch(dma_nth.count()){
        #ifdef DMA1_PRESENT
        case 1:
            return Nth((inst_base - DMA1_Channel1_BASE) / 
                (DMA1_Channel2_BASE - DMA1_Channel1_BASE) + 1);
        #endif

        #ifdef DMA2_PRESENT
        case 2:
            if(inst_base < DMA2_Channel7_BASE){ 
                return Nth(((inst_base - DMA2_Channel1_BASE) / 
                    (DMA2_Channel2_BASE - DMA2_Channel1_BASE)) + 1);
            }else{
                return Nth(((inst_base - DMA2_Channel7_BASE) / 
                    (DMA2_Channel8_BASE - DMA2_Channel7_BASE)) + 7);
            }
        #endif

    }
    __builtin_trap();
}


static constexpr uint32_t _calc_transfer_complete_mask(const Nth dma_nth, const Nth ch_sel_nth){
    switch(dma_nth.count()){
        #ifdef DMA1_PRESENT
        case 1:{
            constexpr size_t EVERY_SHIFT = 4u;
            return (0b10 << (EVERY_SHIFT * (ch_sel_nth.count() - 1)));
        }
        #endif
        #ifdef DMA2_PRESENT
        case 2:
            if(ch_sel_nth.count() <= 7){ 
                constexpr size_t EVERY_SHIFT = 4u;
                return (0b10 << (EVERY_SHIFT * (ch_sel_nth.count() - 1)));
            }else{
                constexpr size_t EVERY_SHIFT = 4u;
                return (0b10 << (EVERY_SHIFT * (ch_sel_nth.count() - 8)));
            }
        #endif
        default:
            break;
    }
    __builtin_trap();
}


static constexpr uint32_t _calc_transfer_onhalf_mask(const Nth dma_nth, const Nth ch_sel_nth){
    switch(dma_nth.count()){
        #ifdef DMA1_PRESENT
        case 1:{
            constexpr size_t EVERY_SHIFT = 4u;
            return (0b100 << (EVERY_SHIFT * (ch_sel_nth.count() - 1)));
        }
        #endif
        #ifdef DMA2_PRESENT
        case 2:
            if(ch_sel_nth.count() <= 7){ 
                constexpr size_t EVERY_SHIFT = 4u;
                return (0b100 << (EVERY_SHIFT * (ch_sel_nth.count() - 1)));
            }else{
                constexpr size_t EVERY_SHIFT = 4u;
                return (0b100 << (EVERY_SHIFT * (ch_sel_nth.count() - 8)));
            }
        #endif
        default:
            break;
    }
    __builtin_trap();
}

void _dma_enable_rcc(const Nth nth, Enable en){
    #ifdef DMA2_PRESENT
    switch(nth.count()){
        case 1:
            RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, (en == EN));
            break;
        case 2:
            RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, (en == EN));
            break;
    }
    #else
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, (en == EN));
    #endif
}

}

#if 0
    #ifdef DMA1_PRESENT
    friend void ::DMA1_Channel1_IRQHandler(void);
    friend void ::DMA1_Channel2_IRQHandler(void);
    friend void ::DMA1_Channel3_IRQHandler(void);
    friend void ::DMA1_Channel4_IRQHandler(void);
    friend void ::DMA1_Channel5_IRQHandler(void);
    friend void ::DMA1_Channel6_IRQHandler(void);
    friend void ::DMA1_Channel7_IRQHandler(void);
    friend void ::DMA1_Channel8_IRQHandler(void);
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
#endif



static constexpr IRQn dma_to_irqn(const Nth dma_nth, const Nth ch_sel_nth){
    switch(dma_nth.count()){
        #ifdef DMA1_PRESENT
        case 1:
            switch(ch_sel_nth.count()){
                case 1: return DMA1_Channel1_IRQn;
                case 2: return DMA1_Channel2_IRQn;
                case 3: return DMA1_Channel3_IRQn;
                case 4: return DMA1_Channel4_IRQn;
                case 5: return DMA1_Channel5_IRQn;
                case 6: return DMA1_Channel6_IRQn;
                case 7: return DMA1_Channel7_IRQn;
                #if defined(DMA1_Channel8)
                case 8: return DMA1_Channel8_IRQn;
                #endif
            }
            break;
        #endif
        #ifdef DMA2_PRESENT
        case 2:
            switch(ch_sel_nth.count()){
                case 1: return DMA2_Channel1_IRQn;
                case 2: return DMA2_Channel2_IRQn;
                case 3: return DMA2_Channel3_IRQn;
                case 4: return DMA2_Channel4_IRQn;
                case 5: return DMA2_Channel5_IRQn;
                case 6: return DMA2_Channel6_IRQn;
                case 7: return DMA2_Channel7_IRQn;
                case 8: return DMA2_Channel8_IRQn;
                case 9: return DMA2_Channel9_IRQn;
                case 10: return DMA2_Channel10_IRQn;
                case 11: return DMA2_Channel11_IRQn;
            }
            break;
        #endif
    }
    __builtin_trap();
}


static volatile uint32_t & get_flag_clear_reg(
    [[maybe_unused]] const Nth dma_nth, 
    [[maybe_unused]] const Nth ch_sel_nth
){
    #if not defined(DMA2_PRESENT)
    return DMA1->INTFCR;
    #else
    if(dma_nth.count() == 2){
        if(ch_sel_nth.count() > 7) return DMA2_EXTEN->INTFCR;
        else return DMA2->INTFCR;
    }else{
        return DMA1->INTFCR;
    }
    #endif
}

static volatile uint32_t & get_flag_state_reg(
    [[maybe_unused]] const Nth dma_nth, 
    [[maybe_unused]] const Nth ch_sel_nth
){
    #if not defined(DMA2_PRESENT)
    return DMA1->INTFR;
    #else
    if(dma_nth.count() == 2){
        if(ch_sel_nth.count() > 7) return DMA2_EXTEN->INTFR;
        else return DMA2->INTFR;
    }else{
        return DMA1->INTFR;
    }
    #endif
}

DmaChannel::DmaChannel(void * inst):
    inst_(inst), 
    dma_nth_(_dma_calc_nth(reinterpret_cast<uint32_t>(inst))),
    ch_sel_nth_(_dma_ch_sel_nth(reinterpret_cast<uint32_t>(inst))),
    flag_clear_reg_(get_flag_clear_reg(dma_nth_, ch_sel_nth_)),
    flag_state_reg_(get_flag_state_reg(dma_nth_, ch_sel_nth_)),
    transfer_complete_mask_(_calc_transfer_complete_mask(dma_nth_, ch_sel_nth_)),
    transfer_onhalf_mask_(_calc_transfer_onhalf_mask(dma_nth_, ch_sel_nth_))
    {;}
    
void DmaChannel::enable_rcc(Enable en){
    _dma_enable_rcc(dma_nth_, en);
}


void DmaChannel::start_transfer(uintptr_t dst_addr, uintptr_t src_addr, const size_t size){

    if(mode_.dst_is_periph()){
        RAL_INST(inst_) -> PADDR.BITS = dst_addr;
        RAL_INST(inst_) -> MADDR.BITS = src_addr;
    }else{
        RAL_INST(inst_) -> PADDR.BITS = src_addr;
        RAL_INST(inst_) -> MADDR.BITS = dst_addr;
    }
    RAL_INST(inst_) -> CNTR.BITS = size;
    clear_pending_flag_and_restart();
}


void DmaChannel::init(const Config & cfg){
    enable_rcc(EN);
    mode_ = cfg.mode;
    DMA_InitTypeDef DMA_InitStructure;

    DMA_InitStructure.DMA_Mode = cfg.mode.is_circular() ? DMA_Mode_Circular : DMA_Mode_Normal;

    switch(DmaDirection(cfg.mode).kind()){
        case DmaDirection::PeriphToBurstMemory :
            DMA_InitStructure.DMA_PeripheralBaseAddr = 0;
            DMA_InitStructure.DMA_MemoryBaseAddr = 0;
            DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
            DMA_InitStructure.DMA_BufferSize = 0;
            DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
            DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
            DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
            break;
            
        case DmaDirection::BurstMemoryToPeriph:
            DMA_InitStructure.DMA_PeripheralBaseAddr = 0;
            DMA_InitStructure.DMA_MemoryBaseAddr = 0;
            DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
            DMA_InitStructure.DMA_BufferSize = 0;
            DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
            DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
            DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
            break;
        case DmaDirection::Synergy:
            DMA_InitStructure.DMA_PeripheralBaseAddr = 0;
            DMA_InitStructure.DMA_MemoryBaseAddr = 0;
            DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
            DMA_InitStructure.DMA_BufferSize = 0;
            DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
            DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
            DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;
            break;
        case DmaDirection::Distribute:
            DMA_InitStructure.DMA_PeripheralBaseAddr = 0;
            DMA_InitStructure.DMA_MemoryBaseAddr = 0;
            DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
            DMA_InitStructure.DMA_BufferSize = 0;
            DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
            DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
            DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;
            break;
    }

    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;

    DMA_InitStructure.DMA_Priority = static_cast<uint32_t>(cfg.priority) << 12;

    DMA_Init(SPL_INST(inst_), &DMA_InitStructure);
}

void DmaChannel::register_nvic(const NvicPriorityCode priority, const Enable en){
    const auto iqrn = dma_to_irqn(dma_nth_, ch_sel_nth_);
    lld::nvic_set_irqn_priority(iqrn, priority);
    lld::nvic_enable_irqn(iqrn, en == EN);
}


[[nodiscard]] bool DmaChannel::is_transfer_complete(){
    uint32_t tempreg = static_cast<uint32_t>(flag_state_reg_);
    return tempreg & transfer_complete_mask_;
}

[[nodiscard]] bool DmaChannel::is_transfer_onhalf(){
    uint32_t tempreg = static_cast<uint32_t>(flag_state_reg_);
    return tempreg & transfer_onhalf_mask_;
}



void DmaChannel::clear_pending_flag_and_restart(){
    flag_clear_reg_ = (transfer_complete_mask_ | transfer_onhalf_mask_);

    RAL_INST(inst_)->CFGR.EN = 1;
}

size_t DmaChannel::pending_count(){
    return static_cast<size_t>(RAL_INST(inst_) -> CNTR.BITS);
}

void DmaChannel::enable_transfer_complete_interrupt(const Enable en){

    //write 1 clear
    flag_clear_reg_ = (transfer_complete_mask_);
    RAL_INST(inst_)->CFGR.TCIE = en == EN;
}

void DmaChannel::enable_transfer_onhalf_interrupt(const Enable en){
    //write 1 clear
    flag_clear_reg_ = (transfer_onhalf_mask_);
    RAL_INST(inst_)->CFGR.HTIE = en == EN;
}

