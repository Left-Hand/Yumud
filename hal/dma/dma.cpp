#include "dma.hpp"
#include "ral/ch32/ch32_common_dma_def.hpp"
#include "core/sdk.hpp"

using namespace ymd;
using namespace ymd::hal;


namespace dma_details{
static constexpr uint8_t calculate_dma_index(const void * inst){
    #ifdef DMA2_PRESENT
    return inst < DMA2_Channel1 ? 1 : 2;
    #else
    return 1;
    #endif
}

static constexpr uint8_t calculate_channel_index(const void * inst){
    uint8_t dma_nth = calculate_dma_index(inst);
    switch(dma_nth){
        #ifdef DMA1_PRESENT
        case 1:
            return (reinterpret_cast<uint32_t>(inst) - DMA1_Channel1_BASE) / 
                (DMA1_Channel2_BASE - DMA1_Channel1_BASE) + 1;
        #endif

        #ifdef DMA2_PRESENT
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
            __builtin_trap();
    }
}


static constexpr uint32_t calculate_done_mask(const void * inst){
    uint8_t dma_nth = calculate_dma_index(inst);
    uint8_t channel_index = calculate_channel_index(inst);
    switch(dma_nth){
        #ifdef DMA1_PRESENT
        case 1:
            return (DMA1_IT_TC1 << ((CTZ(DMA1_IT_TC2) - CTZ(DMA1_IT_TC1)) * (channel_index - 1)));
        #endif
        #ifdef DMA2_PRESENT
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
    uint8_t dma_nth = calculate_dma_index(inst);
    uint8_t channel_index = calculate_channel_index(inst);
    switch(dma_nth){
        #ifdef DMA1_PRESENT
        case 1:
            return (DMA1_IT_HT1 << ((CTZ(DMA1_IT_HT2) - CTZ(DMA1_IT_HT1)) * (channel_index - 1)));
        #endif
        #ifdef DMA2_PRESENT
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
}

namespace ymd::hal{


#ifdef DMA1_PRESENT
DmaChannel dma1_ch1{DMA1_Channel1};
DmaChannel dma1_ch2{DMA1_Channel2};
DmaChannel dma1_ch3{DMA1_Channel3};
DmaChannel dma1_ch4{DMA1_Channel4};
DmaChannel dma1_ch5{DMA1_Channel5};
DmaChannel dma1_ch6{DMA1_Channel6};
DmaChannel dma1_ch7{DMA1_Channel7};
#endif

#ifdef DMA2_PRESENT
DmaChannel dma2_ch1{DMA2_Channel1};
DmaChannel dma2_ch2{DMA2_Channel2};
DmaChannel dma2_ch3{DMA2_Channel3};
DmaChannel dma2_ch4{DMA2_Channel4};
DmaChannel dma2_ch5{DMA2_Channel5};
DmaChannel dma2_ch6{DMA2_Channel6};
DmaChannel dma2_ch7{DMA2_Channel7};
DmaChannel dma2_ch8{DMA2_Channel8};
DmaChannel dma2_ch9{DMA2_Channel9};
DmaChannel dma2_ch10{DMA2_Channel10};
DmaChannel dma2_ch11{DMA2_Channel11};
#endif
}


#define NAME_OF_DMA_XY(x,y) dma##x##_ch##y

#ifdef DMA1_PRESENT
#define DMA1_Inst reinterpret_cast<ral::DMA1_Def *>(DMA1)
#define DMA1_IT_TEMPLATE(y)\
__interrupt void DMA1##_Channel##y##_IRQHandler(void){\
    if(DMA1_Inst->get_transfer_done_flag(y)){\
        NAME_OF_DMA_XY(1,y).accept_interrupt(DmaEvent::TransferComplete);\
        DMA1_Inst->clear_transfer_done_flag(y);\
    }else if(DMA1_Inst->get_transfer_onhalf_flag(y)){\
        NAME_OF_DMA_XY(1,y).accept_interrupt(DmaEvent::HalfTransfer);\
        DMA1_Inst->clear_transfer_onhalf_flag(y);\
    }\
}\

DMA1_IT_TEMPLATE(1);
DMA1_IT_TEMPLATE(2);
DMA1_IT_TEMPLATE(3);
DMA1_IT_TEMPLATE(4);
DMA1_IT_TEMPLATE(5);
DMA1_IT_TEMPLATE(6);
DMA1_IT_TEMPLATE(7);
#endif

#ifdef DMA2_PRESENT
#define DMA2_Inst reinterpret_cast<ral::DMA2_Def *>(DMA2)
#define DMA2_IT_TEMPLATE(y)\
__interrupt void DMA2##_Channel##y##_IRQHandler(void){\
    if(DMA2_Inst->get_transfer_done_flag(y)){\
        NAME_OF_DMA_XY(2,y).accept_interrupt(DmaEvent::TransferComplete);\
        DMA2_Inst->clear_transfer_done_flag(y);\
    }else if(DMA2_Inst->get_transfer_onhalf_flag(y)){\
        NAME_OF_DMA_XY(2,y).accept_interrupt(DmaEvent::HalfTransfer);\
        DMA2_Inst->clear_transfer_onhalf_flag(y);\
    }\
}\

DMA2_IT_TEMPLATE(1);
DMA2_IT_TEMPLATE(2);
DMA2_IT_TEMPLATE(3);
DMA2_IT_TEMPLATE(4);
DMA2_IT_TEMPLATE(5);
DMA2_IT_TEMPLATE(6);
DMA2_IT_TEMPLATE(7);
DMA2_IT_TEMPLATE(8);
DMA2_IT_TEMPLATE(9);
DMA2_IT_TEMPLATE(10);
DMA2_IT_TEMPLATE(11);
#endif

#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SDK_INST(x) (reinterpret_cast<COPY_CONST(inst_,DMA_Channel_TypeDef)>(x))


DmaChannel::DmaChannel(void * inst):
    inst_(inst), 
    done_mask_(dma_details::calculate_done_mask(inst)),
    half_mask_(dma_details::calculate_half_mask(inst)),
    dma_index_(dma_details::calculate_dma_index(inst)),
    channel_index_(dma_details::calculate_channel_index(inst)){;}
    
void DmaChannel::enable_rcc(Enable en){
    #ifdef DMA2_PRESENT
    if(inst_ < DMA2_Channel1){
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, en == EN);
    }else{
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, en == EN);
    }

    #else
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, en == EN);
    #endif
}


void DmaChannel::start_transfer(size_t dst_addr, size_t src_addr, const size_t size){

    if(mode_.dst_is_periph()){
        reinterpret_cast<ral::DMA_CH_Def *>(inst_) -> PADDR = dst_addr;
        reinterpret_cast<ral::DMA_CH_Def *>(inst_) -> MADDR = src_addr;
    }else{
        reinterpret_cast<ral::DMA_CH_Def *>(inst_) -> PADDR = src_addr;
        reinterpret_cast<ral::DMA_CH_Def *>(inst_) -> MADDR = dst_addr;
    }
    reinterpret_cast<ral::DMA_CH_Def *>(inst_) -> CNTR = size;
    resume();
}


void DmaChannel::init(const Config & cfg){
    enable_rcc(EN);
    mode_ = cfg.mode;
    DMA_InitTypeDef DMA_InitStructure;

    DMA_InitStructure.DMA_Mode = cfg.mode.is_circular() ? DMA_Mode_Circular : DMA_Mode_Normal;

    switch(DmaDirection(cfg.mode).kind()){
        case DmaDirection::ToMemory:
            DMA_InitStructure.DMA_PeripheralBaseAddr = 0;
            DMA_InitStructure.DMA_MemoryBaseAddr = 0;
            DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
            DMA_InitStructure.DMA_BufferSize = 0;
            DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
            DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
            DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
            break;
            
        case DmaDirection::ToPeriph:
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

    DMA_Init(SDK_INST(inst_), &DMA_InitStructure);
}

static constexpr IRQn dma_to_irqn(const uint8_t dma_nth, const uint8_t channel_index){
    switch(dma_nth){
        #ifdef DMA1_PRESENT
        case 1:
            switch(channel_index){
                case 1: return DMA1_Channel1_IRQn;
                case 2: return DMA1_Channel2_IRQn;
                case 3: return DMA1_Channel3_IRQn;
                case 4: return DMA1_Channel4_IRQn;
                case 5: return DMA1_Channel5_IRQn;
                case 6: return DMA1_Channel6_IRQn;
                case 7: return DMA1_Channel7_IRQn;
                default: __builtin_unreachable();
            }
        #endif
        #ifdef DMA2_PRESENT
        case 2:
            switch(channel_index){
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
                default: __builtin_unreachable();
            }
        #endif
        default: __builtin_unreachable();
    }

}
void DmaChannel::register_nvic(const NvicPriority priority, const Enable en){
    const auto irq = dma_to_irqn(dma_index_, channel_index_);
    priority.with_irqn(irq).enable(en);
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

[[nodiscard]] bool DmaChannel::is_done(){
    return DMA_GetFlagStatus(done_mask_);
}

void DmaChannel::set_mem_and_periph_bytes(
    const size_t mem_bytes, 
    const size_t periph_bytes
){ 
    // reinterpret_cast<DMA_CH_Def *>(inst_)->CFGR.MSIZE = (mem_bytes) - 1;
    // reinterpret_cast<DMA_CH_Def *>(inst_)->CFGR.PSIZE = (periph_bytes) - 1;
    auto * dma_ch = reinterpret_cast<ral::DMA_CH_Def *>(inst_);
    modify_reg(&dma_ch->CFGR, [&](auto reg){
        reg.MSIZE = (mem_bytes) - 1;
        reg.PSIZE = (periph_bytes) - 1;
        return reg;
    });
}

void DmaChannel::resume(){
    DMA_ClearFlag(done_mask_);
    DMA_ClearFlag(half_mask_);

    DMA_Cmd(SDK_INST(inst_), ENABLE);
}

size_t DmaChannel::remaining(){
    return reinterpret_cast<ral::DMA_CH_Def *>(inst_) -> CNTR;
}

void DmaChannel::enable_done_it(const Enable en){
    DMA_ClearITPendingBit(done_mask_);
    DMA_ITConfig(SDK_INST(inst_), DMA_IT_TC, en == EN);
}

void DmaChannel::enable_half_it(const Enable en){
    DMA_ClearITPendingBit(half_mask_);
    DMA_ITConfig(SDK_INST(inst_), DMA_IT_HT, en == EN);
}

