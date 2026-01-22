#include "can.hpp"
#include "can_filter.hpp"

#include "can_layout.hpp"

#include "hal/sysmisc/nvic/nvic.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/debug/debug.hpp"
#include "ral/can.hpp"
#include "core/sdk.hpp"


using namespace ymd;
using namespace ymd::hal;

using EventCallback = Can::EventCallback;


#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SDK_INST(x) (reinterpret_cast<COPY_CONST(x, CAN_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::CAN_Def)>(x))


#define EMIT_EVENT(x)   if(self.event_callback_ != nullptr) {self.event_callback_(x);}
#define EMIT_EVENT_OR_TRAP(x)   if(self.event_callback_ != nullptr) {self.event_callback_(x);} else{__builtin_trap();}


namespace {
[[maybe_unused]] static Nth _can_to_nth(const void * inst){
    switch(reinterpret_cast<size_t>(inst)){
        #ifdef CAN1_PRESENT
        case CAN1_BASE:
            return Nth(1);
        #endif
        #ifdef CAN2_PRESENT
        case CAN2_BASE:
            return Nth(2);
        #endif
        #ifdef CAN3_PRESENT
        case CAN3_BASE:
            return Nth(3);
        #endif
    }
    __builtin_trap();
}


template<CanRemap REMAP>
[[maybe_unused]] static Gpio _can_to_tx_pin(const void * inst){
    const auto nth = _can_to_nth(inst);
    switch(nth.count()){
        #ifdef CAN1_PRESENT
        case 1:
            return pintag_to_pin<can::tx_pin_t<1, REMAP>>();
        #endif
        #ifdef CAN2_PRESENT
        case 2:
            return pintag_to_pin<can::tx_pin_t<2, REMAP>>();
        #endif
        #ifdef CAN3_PRESENT
        case 3:
            return pintag_to_pin<can::tx_pin_t<3, REMAP>>();
        #endif

    }
    __builtin_trap();
}

template<CanRemap REMAP>
[[maybe_unused]] static Gpio _can_to_rx_pin(const void * inst){
    const auto nth = _can_to_nth(inst);
    switch(nth.count()){
        #ifdef CAN1_PRESENT
        case 1:
            return pintag_to_pin<can::rx_pin_t<1, REMAP>>();
        #endif
        #ifdef CAN2_PRESENT
        case 2:
            return pintag_to_pin<can::rx_pin_t<2, REMAP>>();
        #endif
        #ifdef CAN3_PRESENT
        case 3:
            return pintag_to_pin<can::rx_pin_t<3, REMAP>>();
        #endif

    }
    __builtin_trap();
}

#define DEF_CAN_BIND_PIN_LAYOUTER(name)\
[[maybe_unused]] static Gpio can_to_##name##_pin(const void * inst, const CanRemap remap){\
    switch(remap){\
        case CanRemap::_0: return _can_to_##name##_pin<CanRemap::_0>(inst);\
        case CanRemap::_1: return _can_to_##name##_pin<CanRemap::_1>(inst);\
        case CanRemap::_2: return _can_to_##name##_pin<CanRemap::_2>(inst);\
        case CanRemap::_3: return _can_to_##name##_pin<CanRemap::_3>(inst);\
    }\
    __builtin_trap();\
}\

DEF_CAN_BIND_PIN_LAYOUTER(tx)
DEF_CAN_BIND_PIN_LAYOUTER(rx)

#undef DEF_CAN_BIND_PIN_LAYOUTER
}


namespace {
[[nodiscard]] static constexpr uint32_t _can_tstatr_tme_mask(const CanMailboxIndex mbox_idx){ 
    switch(mbox_idx){
        case CanMailboxIndex::_0: return CAN_TSTATR_TME0;
        case CanMailboxIndex::_1: return CAN_TSTATR_TME1;
        case CanMailboxIndex::_2: return CAN_TSTATR_TME2;
    }
    __builtin_unreachable();
}

[[nodiscard]] static constexpr uint32_t _can_statr_rqcp_mask(const CanMailboxIndex mbox_idx){ 
    switch(mbox_idx){
        case CanMailboxIndex::_0: return CAN_TSTATR_RQCP0;
        case CanMailboxIndex::_1: return CAN_TSTATR_RQCP1;
        case CanMailboxIndex::_2: return CAN_TSTATR_RQCP2;
    }
    __builtin_unreachable();
}

[[nodiscard]] static constexpr uint32_t _can_statr_tkok_mask(const CanMailboxIndex mbox_idx){ 
    switch(mbox_idx){
        case CanMailboxIndex::_0: return CAN_TSTATR_TXOK0;
        case CanMailboxIndex::_1: return CAN_TSTATR_TXOK1;
        case CanMailboxIndex::_2: return CAN_TSTATR_TXOK2;
    }
    __builtin_unreachable();
}

static Option<CanMailboxIndex> _can_get_idle_mailbox_index(void * inst_){
    static constexpr uint32_t ANY_MAILBOX_IDLE_BITMASK = 
        (_can_tstatr_tme_mask(CanMailboxIndex::_0) 
        | _can_tstatr_tme_mask(CanMailboxIndex::_1) 
        | _can_tstatr_tme_mask(CanMailboxIndex::_2));

    const uint32_t tempreg = SDK_INST(inst_)->TSTATR;
    const bool is_any_mailbox_idle = (tempreg & ANY_MAILBOX_IDLE_BITMASK) != 0;
    if(not is_any_mailbox_idle) return None;
    const uint8_t idle_mbox_idx_bits = static_cast<uint8_t>((tempreg & CAN_TSTATR_CODE) >> 24);
    return Some(std::bit_cast<CanMailboxIndex>(idle_mbox_idx_bits));
};

template<CanFifoIndex fifo_idx>
[[nodiscard]] static volatile uint32_t & _can_get_rfifo_reg(void * inst){
    switch(fifo_idx){
        case CanFifoIndex::_0: return SDK_INST(inst)->RFIFO0;
        case CanFifoIndex::_1: return SDK_INST(inst)->RFIFO1;
    }
    __builtin_unreachable();
}

template<uint32_t IT>
[[nodiscard]] static ITStatus _can_get_it_status(void * inst, const uint32_t reg){
    if((reg & IT) != RESET){
        if constexpr(IT == CAN_IT_TME){
            return(SDK_INST(inst)->TSTATR & (CAN_TSTATR_RQCP0|CAN_TSTATR_RQCP1|CAN_TSTATR_RQCP2));  
        }else if constexpr(IT == CAN_IT_FMP0){
            return (SDK_INST(inst)->RFIFO0 & CAN_RFIFO0_FMP0);  
        }else if constexpr(IT == CAN_IT_FF0){
            return (SDK_INST(inst)->RFIFO0 & CAN_RFIFO0_FULL0);  
        }else if constexpr(IT == CAN_IT_FOV0){
            return (SDK_INST(inst)->RFIFO0 & CAN_RFIFO0_FOVR0);  
        }else if constexpr(IT == CAN_IT_FMP1){
            return (SDK_INST(inst)->RFIFO1 & CAN_RFIFO1_FMP1);  
        }else if constexpr(IT == CAN_IT_FF1){
            return (SDK_INST(inst)->RFIFO1 & CAN_RFIFO1_FULL1);  
        }else if constexpr(IT == CAN_IT_FOV1){
            return (SDK_INST(inst)->RFIFO1 & CAN_RFIFO1_FOVR1);  
        }else if constexpr (IT == CAN_IT_WKU)
            return (SDK_INST(inst)->STATR & CAN_STATR_WKUI);  
        else if constexpr (IT == CAN_IT_SLK)
            return (SDK_INST(inst)->STATR & CAN_STATR_SLAKI);  
        else if constexpr (IT == CAN_IT_EWG)
            return (SDK_INST(inst)->ERRSR & CAN_ERRSR_EWGF);  
        else if constexpr (IT == CAN_IT_EPV)
            return (SDK_INST(inst)->ERRSR & CAN_ERRSR_EPVF); 
        else if constexpr (IT == CAN_IT_BOF)
            return (SDK_INST(inst)->ERRSR & CAN_ERRSR_BOFF);  
        else if constexpr (IT == CAN_IT_LEC)
            return (SDK_INST(inst)->ERRSR & CAN_ERRSR_LEC);  
        else if constexpr (IT == CAN_IT_ERR)
            return (SDK_INST(inst)->STATR & CAN_STATR_ERRI); 
    }
    return RESET;
}


template<uint32_t IT>
static void can_clear_it_pending_bit(void * inst)
{
    if constexpr(IT == CAN_IT_TME){
        SDK_INST(inst)->TSTATR = CAN_TSTATR_RQCP0 | CAN_TSTATR_RQCP1 | CAN_TSTATR_RQCP2;  
    }else if constexpr(IT == CAN_IT_FF0){
        SDK_INST(inst)->RFIFO0 = CAN_RFIFO0_FULL0;
    }else if constexpr(IT == CAN_IT_FOV0){
        SDK_INST(inst)->RFIFO0 = CAN_RFIFO0_FOVR0;
    }else if constexpr(IT == CAN_IT_FF1){
        SDK_INST(inst)->RFIFO1 = CAN_RFIFO1_FULL1;
    }else if constexpr(IT == CAN_IT_FOV1){
        SDK_INST(inst)->RFIFO1 = CAN_RFIFO1_FOVR1;
    }else if constexpr(IT == CAN_IT_WKU){
        SDK_INST(inst)->STATR = CAN_STATR_WKUI;
    }else if constexpr(IT == CAN_IT_SLK){
        SDK_INST(inst)->STATR = CAN_STATR_SLAKI;
    }else if constexpr(IT == CAN_IT_EWG){
        SDK_INST(inst)->STATR = CAN_STATR_ERRI;
    }else if constexpr(IT == CAN_IT_EPV){
        SDK_INST(inst)->STATR = CAN_STATR_ERRI; 
    }else if constexpr (IT == CAN_IT_BOF){
        SDK_INST(inst)->STATR = CAN_STATR_ERRI; 
    }else if constexpr (IT == CAN_IT_LEC){
        SDK_INST(inst)->ERRSR = RESET; 
        SDK_INST(inst)->STATR = CAN_STATR_ERRI; 
    }else if constexpr (IT == CAN_IT_ERR){
        SDK_INST(inst)->ERRSR = RESET; 
        SDK_INST(inst)->STATR = CAN_STATR_ERRI; 
	}
}

static void can_enable_rcc(void * inst, const Enable en){
    switch(reinterpret_cast<size_t>(inst)){
        #ifdef CAN1_PRESENT
        case CAN1_BASE:{
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
            return;
        }
        #endif

        #ifdef CAN2_PRESENT
        case CAN2_BASE:{
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
            return;
        }
        #endif

        #ifdef CAN3_PRESENT
        case CAN3_BASE:{
            // TODO: 暂不支持CAN3
            __builtin_trap();
        }
        #endif
    }
    //如果运行到这里 说明调用了预期外的外设 请检查是否正确配置开关宏
    __builtin_trap();
}


static void can_set_remap(void * inst_, const CanRemap remap){
    switch(reinterpret_cast<uint32_t>(inst_)){
        #ifdef CAN1_PRESENT
        case CAN1_BASE:{
            switch(remap){
                case CanRemap::_0:
                    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, DISABLE);
                    return;
                case CanRemap::_1:
                case CanRemap::_2:
                    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
                    return;
                case CanRemap::_3:
                    GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);//for TEST
                    GPIO_PinRemapConfig(GPIO_Remap2_CAN1, ENABLE);
                    return;
            }
        }
        break;
        #endif

        #ifdef CAN2_PRESENT
        case CAN2_BASE:{
            switch(remap){
                case CanRemap::_0:
                    GPIO_PinRemapConfig(GPIO_Remap_CAN2, DISABLE);
                    return;
                case CanRemap::_1:
                    GPIO_PinRemapConfig(GPIO_Remap_CAN2, ENABLE);
                    return;
            }
        }
        break;
        #endif

        #ifdef CAN3_PRESENT
        case CAN3_BASE:{
            //还未实现
            __builtin_trap();
        }
        break;
        #endif
    }
    //如果运行到这里 说明调用了预期外的外设 请检查是否正确配置开关宏
    __builtin_trap();
}


static constexpr uint32_t CAN_RFIFO_FMP_MASK = 0b00'0011;
static constexpr uint32_t CAN_RFIFO_FOM_MASK = 0b10'0000;
static constexpr uint32_t CAN_RFIFO_FFULL_MASK =  0b00'1000;
static constexpr uint32_t CAN_RFIFO_FOV_MASK =  0b01'0000;

}


//CAN发送中断 NVIC优先级
static constexpr NvicPriority CAN_TX_INTERRUPT_NVIC_PRIORITY = {1, 4};

//CAN接收中断0 NVIC优先级
static constexpr NvicPriority CAN_RX0_INTERRUPT_NVIC_PRIORITY = {1, 2};

//CAN接收中断1 NVIC优先级
static constexpr NvicPriority CAN_RX1_INTERRUPT_NVIC_PRIORITY = {1, 2};

//CAN状态改变中断 NVIC优先级
static constexpr NvicPriority CAN_SCE_INTERRUPT_NVIC_PRIORITY = {1, 1};



void Can::init(const Config & cfg){
    set_remap(cfg.remap);
    alter_to_pins(cfg.remap);

    enable_rcc(EN);
    init_interrupts();

    const auto bit_timming_coeffs = [&] -> CanNominalBitTimmingCoeffs{
        const auto & bit_timming = cfg.bit_timming;
        if(bit_timming.is<CanBaudrate>()){
            return (bit_timming.unwrap_as<CanBaudrate>()).to_coeffs(get_aligned_bus_clk_freq());
        }else if(bit_timming.is<CanNominalBitTimmingCoeffs>()){
            return bit_timming.unwrap_as<CanNominalBitTimmingCoeffs>();
        }else{
            //运行到这里说明库组件一致性出现问题 请联系我修复
            __builtin_trap();
        }
    }();


    const CAN_InitTypeDef CAN_InitConf = {
        .CAN_Prescaler = bit_timming_coeffs.prescale,
        .CAN_Mode = std::bit_cast<uint8_t>(cfg.wiring_mode),
        .CAN_SJW = std::bit_cast<uint8_t>(bit_timming_coeffs.swj),
        .CAN_BS1 = std::bit_cast<uint8_t>(bit_timming_coeffs.bs1),
        .CAN_BS2 = std::bit_cast<uint8_t>(bit_timming_coeffs.bs2),

        .CAN_TTCM = DISABLE,
        .CAN_ABOM = ENABLE,
        .CAN_AWUM = DISABLE,
        .CAN_NART = ENABLE,
        .CAN_RFLM = DISABLE,
        .CAN_TXFP = DISABLE,
    };

    if(const auto status = CAN_Init(SDK_INST(inst_), &CAN_InitConf);
        status == CAN_InitStatus_Failed){
        //初始化失败
        __builtin_trap();
    }

}

void Can::deinit(){
    enable_rcc(DISEN);
};

void Can::init_interrupts(){
    const uint32_t it_mask = 
        CAN_IT_TME      //tx done
        | CAN_IT_FMP0   //rx fifo0
        | CAN_IT_FMP1   //rx fifo1

        #ifdef CAN_SCE_ENABLED
        | CAN_IT_ERR 
        | CAN_IT_WKU
        | CAN_IT_SLK 
        | CAN_IT_EWG 
        | CAN_IT_EPV 
        | CAN_IT_BOF
        | CAN_IT_LEC
        #endif
    ;

    CAN_ClearITPendingBit(SDK_INST(inst_), it_mask);
    CAN_ITConfig(SDK_INST(inst_), it_mask, ENABLE);


    switch(reinterpret_cast<uint32_t>(inst_)){
        #ifdef CAN1_PRESENT
        case CAN1_BASE:
            //tx interrupt
            CAN_TX_INTERRUPT_NVIC_PRIORITY.with_irqn(USB_HP_CAN1_TX_IRQn).enable(EN);
            //rx0 interrupt
            CAN_RX0_INTERRUPT_NVIC_PRIORITY.with_irqn(USB_LP_CAN1_RX0_IRQn).enable(EN);
            //rx1 interrupt
            CAN_RX1_INTERRUPT_NVIC_PRIORITY.with_irqn(CAN1_RX1_IRQn).enable(EN);
            //sce interrupt

            #ifdef CAN_SCE_ENABLED
            CAN_SCE_INTERRUPT_NVIC_PRIORITY.with_irqn(CAN1_SCE_IRQn).enable(EN);
            #endif
            break;
        #endif

        #ifdef CAN2_PRESENT
        case CAN2_BASE:
            //tx interrupt
            CAN_TX_INTERRUPT_NVIC_PRIORITY.with_irqn(CAN2_TX_IRQn).enable(EN);
            //rx0 interrupt
            CAN_RX0_INTERRUPT_NVIC_PRIORITY.with_irqn(CAN2_RX0_IRQn).enable(EN);
            //rx1 interrupt
            CAN_RX1_INTERRUPT_NVIC_PRIORITY.with_irqn(CAN2_RX1_IRQn).enable(EN);
            //sce interrupt
            #ifdef CAN_SCE_ENABLED
            CAN_SCE_INTERRUPT_NVIC_PRIORITY.with_irqn(CAN2_SCE_IRQn).enable(EN);
            #endif
            break;
        #endif

        #ifdef CAN3_PRESENT
        case CAN3_BASE:
            //tx interrupt
            CAN_TX_INTERRUPT_NVIC_PRIORITY.with_irqn(CAN3_TX_IRQn).enable(EN);
            //rx0 interrupt
            CAN_RX0_INTERRUPT_NVIC_PRIORITY.with_irqn(CAN3_RX0_IRQn).enable(EN);
            //rx1 interrupt
            CAN_RX1_INTERRUPT_NVIC_PRIORITY.with_irqn(CAN3_RX1_IRQn).enable(EN);
            //sce interrupt
            #ifdef CAN_SCE_ENABLED
            CAN_SCE_INTERRUPT_NVIC_PRIORITY.with_irqn(CAN3_SCE_IRQn).enable(EN);
            #endif
            break;
        #endif
        
        default:
            //如果运行到这里 说明调用了预期外的外设 请检查是否正确配置开关宏
            __builtin_trap();
            break;
    }
}




void Can::transmit(const BxCanFrame & frame, CanMailboxIndex mbox_idx){
    if(size_t(mbox_idx) > 2) __builtin_unreachable();

    auto & mailbox_inst = SDK_INST(inst_)->sTxMailBox[
        static_cast<size_t>(mbox_idx)];

    const uint32_t tempmir = frame.identifier().to_sxx32_reg_bits();
    const uint64_t payload_u64 = frame.payload_u64();


    mailbox_inst.TXMDTR = uint32_t(0xFFFF0000 | (frame.dlc().to_bits() & 0xf));

    //将低四字节装载到txmdlr
    mailbox_inst.TXMDLR = static_cast<uint32_t>(payload_u64);

    //将高四字节装载到txmdhr
    mailbox_inst.TXMDHR = static_cast<uint32_t>(payload_u64 >> 32);

    //有关TXMIR和TXMDTR的描述，请参考芯片数据手册
    //!txmir必须最后填写 因为填写txmir时会导致当前正在填充的报文被发出
    mailbox_inst.TXMIR = tempmir;

}



Result<void, CanLibError> Can::try_write(const BxCanFrame & frame){

    while(true){
        //查找空闲的邮箱
        const auto may_idle_mbox_idx = _can_get_idle_mailbox_index(inst_);
    
        //大概率有空闲邮箱 查找到空闲邮箱后发送
        if(may_idle_mbox_idx.is_some()){
            const auto idle_mbox_idx = may_idle_mbox_idx.unwrap();
            if(tx_fifo_.length() == 0){
                transmit(frame, idle_mbox_idx);
                return Ok();
            }else{
                transmit(tx_fifo_.pop_unchecked(), idle_mbox_idx);
            }
        }else{
            break;
        }
    }

    //如果没找到空闲邮箱 存入软fifo
    if(const auto len = tx_fifo_.try_push(frame);
        len == 0){
        // 如果软fifo已满 则返回错误
        return Err(CanLibError::SoftFifoOverflow);
    }
    return Ok();

}


BxCanFrame Can::read(){
    BxCanFrame frame = BxCanFrame::from_uninitialized();
    if(rx_fifo_.try_pop(frame) == 0)
        __builtin_trap();
    return frame;
}

Option<BxCanFrame> Can::try_read(){
    //如果没有可读的报文 返回空
    if(rx_fifo_.length() == 0) return None;
    //弹出可读的报文
    return Some(rx_fifo_.pop_unchecked());
}



BxCanFrame Can::receive(const CanFifoIndex fifo_idx){
    const auto & mailbox = SDK_INST(inst_)->sFIFOMailBox[std::bit_cast<uint8_t>(fifo_idx)];
    const uint32_t rxmir = mailbox.RXMIR;
    const uint32_t rxmdtr = mailbox.RXMDTR;

    //获取载荷长度
    const uint8_t dlc_bits = rxmdtr & (0x0F);

    //将低四位的和高四位拼接为完整的8x8的载荷
    const uint64_t payload_u64 = 
        static_cast<uint64_t>(static_cast<uint32_t>(mailbox.RXMDLR)) 
        | (static_cast<uint64_t>(static_cast<uint32_t>(mailbox.RXMDHR)) << 32);

    return BxCanFrame::from_sxx32_regs(rxmir, payload_u64, dlc_bits);
}


size_t Can::free_capacity(){
    return tx_fifo_.free_capacity();
}

size_t Can::available(){
    return rx_fifo_.length();
}

void Can::alter_to_pins(const CanRemap remap){
    can_to_tx_pin(inst_, remap).afpp();
    can_to_rx_pin(inst_, remap).afpp();
}



void Can::enable_rcc(const Enable en){
    can_enable_rcc(inst_, en);
}

void Can::set_remap(const CanRemap remap){
    can_set_remap(inst_, remap);
}



void Can::enable_hw_retransmit(const Enable en){
    if(en == EN)  SDK_INST(inst_)->CTLR &= ~CAN_CTLR_NART;
    else    SDK_INST(inst_)->CTLR |=  CAN_CTLR_NART;
}

uint32_t Can::get_aligned_bus_clk_freq(){
    //所有的CAN外设都使用APB1时钟
    return sys::clock::get_apb1_clk_freq();
}

uint8_t Can::get_rx_errcnt(){
    return static_cast<uint8_t>(SDK_INST(inst_)->ERRSR >> 24);
}

uint8_t Can::get_tx_errcnt(){
    return static_cast<uint8_t>(SDK_INST(inst_)->ERRSR >> 16);
}

Option<Can::Error> Can::last_error(){
    const uint8_t bits = static_cast<uint8_t>(RAL_INST(inst_)->ERRSR.LEC & 0xff);
    if(bits == 0) return None;
    return Some(std::bit_cast<Can::Error>(bits));
}

bool Can::is_tranmitting(){
    return bool(SDK_INST(inst_)->STATR & CAN_STATR_TXM);
}

bool Can::is_receiving(){
    return bool(SDK_INST(inst_)->STATR & CAN_STATR_RXM);
}

bool Can::is_busoff(){
    return bool(SDK_INST(inst_)->ERRSR & CAN_ERRSR_BOFF);
}

void Can::abort_transmit(const CanMailboxIndex mbox_idx){
    SDK_INST(inst_)->TSTATR = _can_statr_rqcp_mask(mbox_idx);
}

void Can::abort_all_transmits(){
    static constexpr uint32_t MASK = 
        _can_statr_rqcp_mask(CanMailboxIndex::_0)  
        | _can_statr_rqcp_mask(CanMailboxIndex::_1)  
        | _can_statr_rqcp_mask(CanMailboxIndex::_2)
    ;
    SDK_INST(inst_)->TSTATR = MASK;
}

void Can::enable_rxfifo_lock(const Enable en){
    if(en == EN) SDK_INST(inst_)->CTLR |= CAN_CTLR_RFLM;
    else SDK_INST(inst_)->CTLR &= ~CAN_CTLR_RFLM;
}

void Can::enable_index_priority(const Enable en){
    if(en == EN) SDK_INST(inst_)->CTLR |= CAN_CTLR_TXFP;
    else SDK_INST(inst_)->CTLR &= ~CAN_CTLR_TXFP;
}

#if 0
void CanInterruptDispatcher::isr_tx(Can & self){
    volatile uint32_t & tstatr_reg = SDK_INST(self.inst_)->TSTATR;
    const auto temp_tstatr = tstatr_reg;
    //遍历每个邮箱
    auto iter_mailbox = [&]<CanMailboxIndex mbox_idx>(){
        static constexpr uint32_t TSTATR_TME_MASK = can_tstatr_tme_mask(mbox_idx);
        static constexpr uint32_t TSTATR_RQCP_MASK = can_statr_rqcp_mask(mbox_idx);
        static constexpr uint32_t TSTATR_RXOK_MASK = can_statr_tkok_mask(mbox_idx);
    
        switch(temp_tstatr & (TSTATR_TME_MASK | TSTATR_RQCP_MASK | TSTATR_RXOK_MASK)){
            case 0:
                //pending
                break;
            case TSTATR_TME_MASK | TSTATR_RQCP_MASK | 0:
                //发送失败
                {
                    const auto tx_ev = hal::CanTransmitEvent{
                        .kind = CanTransmitEvent::Kind::Failed,
                        .mbox_idx = mbox_idx
                    };
                    const auto ev = CanEvent::from(tx_ev);
                    EMIT_EVENT(ev);
                }

                //清除发送标志位
                tstatr_reg = TSTATR_RQCP_MASK;
                break;
            case TSTATR_TME_MASK | TSTATR_RQCP_MASK | TSTATR_RXOK_MASK:
                //发送成功
                {
                    const auto tx_ev = hal::CanTransmitEvent{
                        .kind = CanTransmitEvent::Kind::Success,
                        .mbox_idx = mbox_idx
                    };
                    const auto ev = CanEvent::from(tx_ev);
                    EMIT_EVENT(ev);
                }

                //清除发送标志位
                tstatr_reg = TSTATR_RQCP_MASK;
                break;
        }
    };

    iter_mailbox.template operator() < CanMailboxIndex::_0 > ();
    iter_mailbox.template operator() < CanMailboxIndex::_1 > ();
    iter_mailbox.template operator() < CanMailboxIndex::_2 > ();


    self.poll_backup_fifo();
}

#else

void CanInterruptDispatcher::isr_tx(Can & self){
    volatile uint32_t & tstatr_reg = SDK_INST(self.inst_)->TSTATR;
    const auto temp_tstatr = tstatr_reg;
    //遍历每个邮箱

    auto iter_mailbox = [&]<CanMailboxIndex mbox_idx>() __attribute__((always_inline)){
        static constexpr uint32_t TSTATR_TME_MASK = _can_tstatr_tme_mask(mbox_idx);
        static constexpr uint32_t TSTATR_RQCP_MASK = _can_statr_rqcp_mask(mbox_idx);
        static constexpr uint32_t TSTATR_RXOK_MASK = _can_statr_tkok_mask(mbox_idx);
        static constexpr uint32_t READY_MASK = TSTATR_TME_MASK | TSTATR_RQCP_MASK;

        if((temp_tstatr & READY_MASK) != READY_MASK){
            //not ready
            return;
        }

        CanTransmitEvent::Kind ev_kind = (temp_tstatr & TSTATR_RXOK_MASK) ? 
            CanTransmitEvent::Kind::Success : CanTransmitEvent::Kind::Failed;

        const auto tx_ev = hal::CanTransmitEvent{
            .kind = ev_kind,
            .mbox_idx = mbox_idx
        };
        const auto ev = CanEvent::from(tx_ev);
        EMIT_EVENT(ev);

        //清除发送标志位
        tstatr_reg = TSTATR_RQCP_MASK;

    };

    iter_mailbox.template operator() < CanMailboxIndex::_0 > ();
    iter_mailbox.template operator() < CanMailboxIndex::_1 > ();
    iter_mailbox.template operator() < CanMailboxIndex::_2 > ();


    self.poll_backup_fifo();
}

#endif

void Can::poll_backup_fifo(){
    auto & self = *this;
    if(self.tx_fifo_.length() == 0) return;
    const auto may_idle_mailbox = _can_get_idle_mailbox_index(self.inst_);
    if(may_idle_mailbox.is_none()) return;
    self.transmit(self.tx_fifo_.pop_unchecked(), may_idle_mailbox.unwrap());
}

void CanInterruptDispatcher::isr_rx0(Can & can){
    CanInterruptDispatcher::isr_rx(can, 
        _can_get_rfifo_reg<CanFifoIndex::_0>(can.inst_), 
        CanFifoIndex::_0
    );
}

void CanInterruptDispatcher::isr_rx1(Can & can){
    CanInterruptDispatcher::isr_rx(can, 
        _can_get_rfifo_reg<CanFifoIndex::_1>(can.inst_), 
        CanFifoIndex::_1
    );
}

void CanInterruptDispatcher::isr_rx(Can & self, volatile uint32_t & rfifo_reg, const CanFifoIndex fifo_idx){
    const uint32_t temp_rfifo_reg = rfifo_reg;

    if(temp_rfifo_reg & CAN_RFIFO_FFULL_MASK){
        //rfifo满
        {
            const auto rx_ev = hal::CanReceiveEvent{
                .kind = CanReceiveEvent::Kind::Full,
                .fifo_idx = fifo_idx
            };
            const auto ev = CanEvent::from(rx_ev);
            EMIT_EVENT_OR_TRAP(ev)
        }
        rfifo_reg = CAN_RFIFO_FFULL_MASK;
    }
    
    if(temp_rfifo_reg & CAN_RFIFO_FOV_MASK){
        ///rfifo溢出
        {
            const auto rx_ev = hal::CanReceiveEvent{
                .kind = CanReceiveEvent::Kind::Overrun,
                .fifo_idx = fifo_idx
            };
            const auto ev = CanEvent::from(rx_ev);
            EMIT_EVENT_OR_TRAP(ev)
        }
        rfifo_reg = CAN_RFIFO_FOV_MASK;
    }

    //注意这里是判断fmp掩码 但是清零的是fom
    if (temp_rfifo_reg & CAN_RFIFO_FMP_MASK){
        //收到新的报文
        {
            //TODO 改为异步sink
            (void)self.rx_fifo_.try_push(self.receive(fifo_idx));
        }

        {
            const auto rx_ev = hal::CanReceiveEvent{
                .kind = CanReceiveEvent::Kind::Pending,
                .fifo_idx = fifo_idx
            };
            const auto ev = CanEvent::from(rx_ev);
            EMIT_EVENT(ev)
        }
        //已读这个报文
        rfifo_reg = CAN_RFIFO_FOM_MASK;
    }
}

void CanInterruptDispatcher::isr_sce(Can & self){
    void * inst = self.inst_;
    const uint32_t temp_inten_reg = SDK_INST(inst)->INTENR;

    auto flag_bits = hal::CanStatusFlag::zero();

    if (_can_get_it_status<CAN_IT_WKU>(SDK_INST(inst), temp_inten_reg)) {
        // Handle Wake-up interrupt
        // 唤醒中断
        flag_bits.wakeup = 1;
        can_clear_it_pending_bit<CAN_IT_WKU>(SDK_INST(inst));
    } 
    
    if (_can_get_it_status<CAN_IT_SLK>(SDK_INST(inst), temp_inten_reg)) {
        // Handle Sleep acknowledge interrupt
        // 睡眠确认中断
        flag_bits.sleep_acknowledge = 1;
        can_clear_it_pending_bit<CAN_IT_SLK>(SDK_INST(inst));
    } 
    
    if (_can_get_it_status<CAN_IT_ERR>(SDK_INST(inst), temp_inten_reg)) {
        // Handle Error interrupt
        // 错误中断
        flag_bits.error = 1;
        can_clear_it_pending_bit<CAN_IT_ERR>(SDK_INST(inst));
    } 
    
    if (_can_get_it_status<CAN_IT_EWG>(SDK_INST(inst), temp_inten_reg)) {
        // Handle Error warning interrupt
        // 主动错误中断
        flag_bits.error_warning = 1;
        can_clear_it_pending_bit<CAN_IT_EWG>(SDK_INST(inst));
    } 
    
    if (_can_get_it_status<CAN_IT_EPV>(SDK_INST(inst), temp_inten_reg)) {
        // Handle Error passive interrupt
        // 被动错误中断
        flag_bits.error_passive = 1;
        can_clear_it_pending_bit<CAN_IT_EPV>(SDK_INST(inst));
    } 
    
    if (_can_get_it_status<CAN_IT_BOF>(SDK_INST(inst), temp_inten_reg)) {
        // Handle Bus-off interrupt
        // 掉线中断
        flag_bits.bus_off = 1;
        can_clear_it_pending_bit<CAN_IT_BOF>(SDK_INST(inst));
    } 
    
    if (_can_get_it_status<CAN_IT_LEC>(SDK_INST(inst), temp_inten_reg)) {
        // Handle Last error code interrupt
        // 错误码中断
        flag_bits.last_error_code = 1;
        can_clear_it_pending_bit<CAN_IT_LEC>(SDK_INST(inst));
    }

    EMIT_EVENT(CanEvent::from(flag_bits));
}

namespace ymd::hal{

#ifdef CAN1_PRESENT
Can can1 = Can{CAN1};
#endif

#ifdef CAN2_PRESENT
Can can2 = Can{CAN2};
#endif

#ifdef CAN3_PRESENT
Can can3 = Can{CAN3};
#endif
}



extern "C"{
#ifdef CAN1_PRESENT
__interrupt void USB_HP_CAN1_TX_IRQHandler(){
    CanInterruptDispatcher::isr_tx(can1);
}

__interrupt void USB_LP_CAN1_RX0_IRQHandler() {
    CanInterruptDispatcher::isr_rx0(can1);
}

__interrupt void CAN1_RX1_IRQHandler(){
    CanInterruptDispatcher::isr_rx1(can1);
}


#ifdef CAN_SCE_ENABLED
__interrupt void CAN1_SCE_IRQHandler(){
    CanInterruptDispatcher::isr_sce(can1);
}
#endif
#endif

#ifdef CAN2_PRESENT
__interrupt void CAN2_TX_IRQHandler(){
    CanInterruptDispatcher::isr_tx(can2);
}

__interrupt void CAN2_RX0_IRQHandler(){
    CanInterruptDispatcher::isr_rx0(can2);
}

__interrupt void CAN2_RX1_IRQHandler(){
    CanInterruptDispatcher::isr_rx1(can2);
}

#ifdef CAN_SCE_ENABLED
__interrupt void CAN2_SCE_IRQHandler(){
    CanInterruptDispatcher::isr_sce(can2);
}

#endif
#endif

#ifdef CAN3_PRESENT
__interrupt void CAN3_TX_IRQHandler(){
    CanInterruptDispatcher::isr_tx(can3);
}

__interrupt void CAN3_RX0_IRQHandler(){
    CanInterruptDispatcher::isr_rx0(can3);
}

__interrupt void CAN3_RX1_IRQHandler(){
    CanInterruptDispatcher::isr_rx1(can3);
}

#ifdef CAN_SCE_ENABLED
__interrupt void CAN3_SCE_IRQHandler(){
    CanInterruptDispatcher::isr_sce(can3);
}

#endif
#endif
}