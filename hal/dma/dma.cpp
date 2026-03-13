#include "dma.hpp"
#include "ral/ch32/ch32_common_dma_def.hpp"
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
            constexpr size_t EVERY_SHIFT = (__builtin_ctz(DMA1_IT_TC2) - __builtin_ctz(DMA1_IT_TC1));
            return (DMA1_IT_TC1 << (EVERY_SHIFT * (ch_sel_nth.count() - 1)));
        }
        #endif
        #ifdef DMA2_PRESENT
        case 2:
            if(ch_sel_nth.count() <= 7){ 
                constexpr size_t EVERY_SHIFT = (__builtin_ctz(DMA2_IT_TC2) - __builtin_ctz(DMA2_IT_TC1));
                return ((uint32_t)(DMA2_IT_TC1 & 0xff) << (EVERY_SHIFT * (ch_sel_nth.count() - 1))) | (uint32_t)(0x10000000);
            }else{
                constexpr size_t EVERY_SHIFT = (__builtin_ctz(DMA2_IT_TC9) - __builtin_ctz(DMA2_IT_TC8));
                return ((uint32_t)(DMA2_IT_TC8 & 0xff) << (EVERY_SHIFT * (ch_sel_nth.count() - 8))) | (uint32_t)(0x20000000);
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
            constexpr size_t EVERY_SHIFT = (__builtin_ctz(DMA1_IT_HT2) - __builtin_ctz(DMA1_IT_HT1));
            return (DMA1_IT_HT1 << (EVERY_SHIFT * (ch_sel_nth.count() - 1)));
        }
        #endif
        #ifdef DMA2_PRESENT
        case 2:
            if(ch_sel_nth.count() <= 7){ 
                constexpr size_t EVERY_SHIFT = ((__builtin_ctz(DMA2_IT_HT2) - __builtin_ctz(DMA2_IT_HT1)));
                return ((uint32_t)(DMA2_IT_HT1 & 0xff) << (EVERY_SHIFT * (ch_sel_nth.count() - 1))) | (uint32_t)(0x10000000);
            }else{
                constexpr size_t EVERY_SHIFT = ((__builtin_ctz(DMA2_IT_HT9) - __builtin_ctz(DMA2_IT_HT8)));
                return ((uint32_t)(DMA2_IT_HT8 & 0xff) << (EVERY_SHIFT * (ch_sel_nth.count() - 8))) | (uint32_t)(0x20000000);
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

template<typename T, typename Fn>
static inline void modify_reg(volatile T* reg, Fn&& fn) {
    static_assert(std::is_trivially_copyable_v<T>, 
        "T must be trivially copyable for register operations");
    
    // 读取并转换
    T temp = std::bit_cast<T>(*const_cast<const T*>(reg));
    
    // 应用修改
    temp = fn(temp);
    
    // 写回
    *const_cast<T*>(reg) = temp;
}



DmaChannel::DmaChannel(void * inst):
    inst_(inst), 
    dma_nth_(_dma_calc_nth(reinterpret_cast<uint32_t>(inst))),
    ch_sel_nth_(_dma_ch_sel_nth(reinterpret_cast<uint32_t>(inst))),
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

void DmaChannel::register_nvic(const NvicPriority priority, const Enable en){
    const auto iqrn = dma_to_irqn(dma_nth_, ch_sel_nth_);
    priority.with_irqn(iqrn).enable(en);
}


[[nodiscard]] bool DmaChannel::is_transfer_complete(){
    return DMA_GetFlagStatus(transfer_complete_mask_);
}

void DmaChannel::set_mem_and_periph_wordsize(
    const WordSize mem_wordsize, 
    const WordSize periph_wordsize
){ 
    auto * dma_ch = RAL_INST(inst_);
    modify_reg(&dma_ch->CFGR, [&](auto reg){
        reg.MSIZE = static_cast<uint8_t>(mem_wordsize);
        reg.PSIZE = static_cast<uint8_t>(periph_wordsize);
        return reg;
    });
}

void DmaChannel::clear_pending_flag_and_restart(){
    DMA_ClearFlag(transfer_complete_mask_ | transfer_onhalf_mask_);

    DMA_Cmd(SPL_INST(inst_), ENABLE);
}

size_t DmaChannel::pending_count(){
    return RAL_INST(inst_) -> CNTR.BITS;
}


#ifdef DMA2

static void my_DMA_ClearITPendingBit(uint32_t DMAy_IT)
{
    if((DMAy_IT & 0x10000000) == 0x10000000)
    {
        DMA2->INTFCR = DMAy_IT;
    }
    else if((DMAy_IT & 0x20000000) == 0x20000000)
    {
        DMA2_EXTEN->INTFCR = DMAy_IT;
    }
    else
    {
        DMA1->INTFCR = DMAy_IT;
    }
}

#else

static void my_DMA_ClearITPendingBit(uint32_t DMAy_IT)
{
    DMA1->INTFCR = DMAy_IT;
}

#endif

static void my_DMA_ITConfig(DMA_Channel_TypeDef *DMAy_Channelx, uint32_t DMA_IT, FunctionalState NewState)
{
    if(NewState != DISABLE)
    {
        DMAy_Channelx->CFGR |= DMA_IT;
    }
    else
    {
        DMAy_Channelx->CFGR &= ~DMA_IT;
    }
}


void DmaChannel::enable_transfer_complete_interrupt(const Enable en){
    my_DMA_ClearITPendingBit(transfer_complete_mask_);
    my_DMA_ITConfig(SPL_INST(inst_), DMA_IT_TC, (en == EN));
}

void DmaChannel::enable_transfer_onhalf_interrupt(const Enable en){
    my_DMA_ClearITPendingBit(transfer_onhalf_mask_);
    my_DMA_ITConfig(SPL_INST(inst_), DMA_IT_HT, (en == EN));
}

