#include "can.hpp"
#include "can_filter.hpp"

#include "can_layout.hpp"

#include "hal/nvic/nvic.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/debug/debug.hpp"
#include "ral/can.hpp"
#include "core/sdk.hpp"
using namespace ymd;
using namespace ymd::hal;

using Callback = Can::Callback;

namespace ymd::hal::can{
// https://docs.rs/embassy-stm32/latest/embassy_stm32/can/config/enum.TxBufferMode.html
enum class TxBufferMode:uint8_t {
    // TX FIFO operation - In this mode CAN frames are trasmitted strictly in write order.
    Fifo,
    // TX priority queue operation - In this mode CAN frames are transmitted according to CAN priority.
    Priority,
};

#if 0
struct FdCanConfig final {
    // Nominal Bit Timings
    CanNominalBitTimming nbtr;
    // (Variable) Data Bit Timings
    DataBitTiming dbtr;
    // Enables or disables automatic retransmission of messages

    // If this is enabled, the CAN peripheral will automatically try to retransmit each frame util it can be sent. Otherwise, it will try only once to send each frame.

    // Automatic retransmission is enabled by default.
    Enable automatic_retransmit;

    // The transmit pause feature is intended for use in CAN systems where the CAN message 
    // identifiers are permanently specified to specific values and cannot easily be changed.

    // These message identifiers can have a higher CAN arbitration priority than other defined messages, 
    // while in a specific application their relative arbitration priority must be inverse.

    // This may lead to a case where one ECU sends a burst of CAN messages that cause another ECU CAN 
    // messages to be delayed because that other messages have a lower CAN arbitration priori
    Enable transmit_pause;

    // Enabled or disables the pausing between transmissions
    // This feature looses up burst transmissions coming from a single node and it protects against 
    // “babbling idiot” scenarios where the application program erroneously requests too many transmissions.
    FrameTransmissionConfig frame_transmit;

    // Non Isoe Mode If this is set, the FDCAN uses the CAN FD frame format as specified by the 
    // Bosch CAN FD Specification V1.0.
    Enable non_iso_mode;

    // Edge Filtering: Two consecutive dominant tq required to detect an edge for hard synchronization
    Enable edge_filtering;

    // Enables protocol exception handling
    Enable protocol_exception_handling;

    // Sets the general clock divider for this FdCAN instance
    ClockDivider clock_divider;

    // Sets the timestamp source
    TimestampSource timestamp_source;

    // Configures the Global Filter
    GlobalFilter global_filter;

    // TX buffer mode (FIFO or priority queue)
    TxBufferMode tx_buffer_mode;
};
#endif
}



#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SDK_INST(x) (reinterpret_cast<COPY_CONST(x, CAN_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::CAN_Def)>(x))



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

static constexpr uint32_t can_tstatr_tme_mask(const CanMailboxIndex mbox_idx){ 
    switch(mbox_idx){
        case CanMailboxIndex::_0: return CAN_TSTATR_TME0;
        case CanMailboxIndex::_1: return CAN_TSTATR_TME1;
        case CanMailboxIndex::_2: return CAN_TSTATR_TME2;
    }
    __builtin_unreachable();
}

static constexpr uint32_t can_statr_rqcp_mask(const CanMailboxIndex mbox_idx){ 
    switch(mbox_idx){
        case CanMailboxIndex::_0: return CAN_TSTATR_RQCP0;
        case CanMailboxIndex::_1: return CAN_TSTATR_RQCP1;
        case CanMailboxIndex::_2: return CAN_TSTATR_RQCP2;
    }
    __builtin_unreachable();
}

static constexpr uint32_t can_statr_tkok_mask(const CanMailboxIndex mbox_idx){ 
    switch(mbox_idx){
        case CanMailboxIndex::_0: return CAN_TSTATR_TXOK0;
        case CanMailboxIndex::_1: return CAN_TSTATR_TXOK1;
        case CanMailboxIndex::_2: return CAN_TSTATR_TXOK2;
    }
    __builtin_unreachable();
}

static constexpr uint32_t can_rfifo_fmp_mask(const CanFifoIndex fifo_idx){ 
    switch(fifo_idx){
        case CanFifoIndex::_0: return CAN_RFIFO0_FMP0;
        case CanFifoIndex::_1: return CAN_RFIFO1_FMP1;
    }
    __builtin_unreachable();
}

static constexpr uint32_t can_rfifo_fom_mask(const CanFifoIndex fifo_idx){ 
    switch(fifo_idx){
        case CanFifoIndex::_0: return CAN_RFIFO0_RFOM0;
        case CanFifoIndex::_1: return CAN_RFIFO1_RFOM1;
    }
    __builtin_unreachable();
}

static constexpr uint32_t can_rfifo_ff_mask(const CanFifoIndex fifo_idx){ 
    switch(fifo_idx){
        case CanFifoIndex::_0: return CAN_RFIFO0_FULL0;
        case CanFifoIndex::_1: return CAN_RFIFO1_FULL1;
    }
    __builtin_unreachable();
}

static constexpr uint32_t can_rfifo_fov_mask(const CanFifoIndex fifo_idx){ 
    switch(fifo_idx){
        case CanFifoIndex::_0: return CAN_RFIFO0_FOVR0;
        case CanFifoIndex::_1: return CAN_RFIFO1_FOVR1;
    }
    __builtin_unreachable();
}


static Option<CanMailboxIndex> can_get_idle_mailbox_index(void * inst_){
    const uint32_t tempreg = SDK_INST(inst_)->TSTATR;
    static constexpr uint32_t ANY_MAILBOX_IDLE_BITMASK = (CAN_TSTATR_TME0 | CAN_TSTATR_TME1 | CAN_TSTATR_TME2);
    const auto is_any_mailbox_idle = (tempreg & ANY_MAILBOX_IDLE_BITMASK) != 0;
    if(not is_any_mailbox_idle) return None;
    return Some(std::bit_cast<CanMailboxIndex>(uint8_t((tempreg & CAN_TSTATR_CODE) >> 24)));
};


template<uint32_t IT>
static ITStatus MY_CAN_GetITStatus(const uint32_t reg, void * inst){
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

    switch(reinterpret_cast<size_t>(SDK_INST(inst_))){
        #ifdef CAN1_PRESENT
        case CAN1_BASE:
            //tx interrupt
            NvicPriority{1, 7}.with_irqn(USB_HP_CAN1_TX_IRQn).enable(EN);
            //rx0 interrupt
            NvicPriority{1, 5}.with_irqn(USB_LP_CAN1_RX0_IRQn).enable(EN);
            //rx1 interrupt
            NvicPriority{1, 6}.with_irqn(CAN1_RX1_IRQn).enable(EN);
            //sce interrupt

            #ifdef CAN_SCE_ENABLED
            NvicPriority{1, 2}.with_irqn(CAN1_SCE_IRQn).enable(EN);
            #endif
            break;
        #endif

        #ifdef CAN2_PRESENT
        case CAN2_BASE:
            //tx interrupt
            NvicPriority{1, 6}.with_irqn(CAN2_TX_IRQn).enable(EN);
            //rx0 interrupt
            NvicPriority{1, 4}.with_irqn(CAN2_RX0_IRQn).enable(EN);
            //rx1 interrupt
            NvicPriority{1, 5}.with_irqn(CAN2_RX1_IRQn).enable(EN);
            //sce interrupt
            #ifdef CAN_SCE_ENABLED
            NvicPriority{1, 2}.with_irqn(CAN2_SCE_IRQn).enable(EN);
            #endif
            break;
        #endif
        
        default:
            __builtin_trap();
            break;
    }
}



void Can::alter_to_pins(const CanRemap remap){
    can_to_tx_pin(inst_, remap).afpp();
    can_to_rx_pin(inst_, remap).afpp();
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
    }

    __builtin_trap();
}


static void can_set_remap(void * inst_, const CanRemap remap){
    switch(reinterpret_cast<size_t>(SDK_INST(inst_))){
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
    }
    __builtin_trap();
}

void Can::enable_rcc(const Enable en){
    can_enable_rcc(inst_, en);
}

void Can::set_remap(const CanRemap remap){
    can_set_remap(inst_, remap);
}


void Can::init(const Config & cfg){
    enable_rcc(EN);
    set_remap(cfg.remap);
    alter_to_pins(cfg.remap);


    const auto bit_timming_coeffs = [&] -> CanNominalBitTimmingCoeffs{
        const auto & bit_timming = cfg.bit_timming;
        if(bit_timming.is<CanBaudrate>()){
            return (bit_timming.unwrap_as<CanBaudrate>()).to_coeffs(get_aligned_bus_clk_freq());
        }else if(bit_timming.is<CanNominalBitTimmingCoeffs>()){
            return bit_timming.unwrap_as<CanNominalBitTimmingCoeffs>();
        }else{
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

    CAN_Init(SDK_INST(inst_), &CAN_InitConf);
    init_interrupts();
}


void Can::transmit(const BxCanFrame & frame, CanMailboxIndex mbox_idx){
    const uint32_t tempmir = frame.identifier().to_bits();
    const uint64_t data = frame.payload_u64();
    auto & mailbox_inst = SDK_INST(inst_)->sTxMailBox[
        static_cast<size_t>(size_t(mbox_idx))];

    mailbox_inst.TXMDLR = data & UINT32_MAX;
    mailbox_inst.TXMDHR = data >> 32;

    mailbox_inst.TXMDTR = uint32_t(0xFFFF0000 | frame.length());
    mailbox_inst.TXMIR = tempmir;
}


void Can::enable_hw_retransmit(const Enable en){
    if(en == EN)  SDK_INST(inst_)->CTLR &= ~CAN_CTLR_NART;
    else    SDK_INST(inst_)->CTLR |=  CAN_CTLR_NART;
}

Result<void, CanLibError> Can::try_write(const BxCanFrame & frame){
    const auto may_idle_mbox_index = can_get_idle_mailbox_index(inst_);

    //大概率有空闲邮箱 查找到空闲邮箱后发送
    if(may_idle_mbox_index.is_some()){
        const auto idle_mbox_index = may_idle_mbox_index.unwrap();
        transmit(frame, idle_mbox_index);
        return Ok();
    }

    //如果没找到空闲邮箱 存入软fifo

    
    if(const auto len = tx_fifo_.try_push(frame);
        len == 0){
        // 如果软fifo已满 则返回错误
        return Err(CanLibError::SoftFifoOverflow);
    }
    return Ok();

}

uint32_t Can::get_aligned_bus_clk_freq(){
    return sys::clock::get_apb1_clk_freq();
}


BxCanFrame Can::read(){

    BxCanFrame frame = BxCanFrame::from_uninitialized();
    if(rx_fifo_.try_pop(frame) == 0)
        __builtin_trap();
    return frame;
}

size_t Can::available(){
    return rx_fifo_.length();
}

uint8_t Can::get_rx_errcnt(){
    return SDK_INST(inst_)->ERRSR >> 24;
}

uint8_t Can::get_tx_errcnt(){
    return SDK_INST(inst_)->ERRSR >> 16;
}

Option<Can::Exception> Can::last_exception(){
    const uint8_t bits = CAN_GetLastErrorCode(SDK_INST(inst_));
    if(bits == 0) return None;
    return Some(std::bit_cast<Can::Exception>(bits));
}

bool Can::is_tranmitting(){
    return bool(SDK_INST(inst_)->STATR & CAN_STATR_TXM);
}

bool Can::is_receiving(){
    return bool(SDK_INST(inst_)->STATR & CAN_STATR_RXM);
}

bool Can::is_busoff(){
    return SDK_INST(inst_)->ERRSR & CAN_ERRSR_BOFF;
}

void Can::cancel_transmit(const CanMailboxIndex mbox_idx){
    CAN_CancelTransmit(SDK_INST(inst_), std::bit_cast<uint8_t>(mbox_idx));
}

void Can::cancel_all_transmits(){
    for(uint8_t i = 0; i < 3; i++) 
        CAN_CancelTransmit(SDK_INST(inst_), i);
    SDK_INST(inst_)->TSTATR = SDK_INST(inst_)->TSTATR | 
        (CAN_TSTATR_ABRQ0 | CAN_TSTATR_ABRQ1 | CAN_TSTATR_ABRQ2);
}

void Can::enable_fifo_lock(const Enable en){
    if(en == EN) SDK_INST(inst_)->CTLR |= CAN_CTLR_RFLM;
    else SDK_INST(inst_)->CTLR &= ~CAN_CTLR_RFLM;
}

void Can::enable_index_priority(const Enable en){
    if(en == EN) SDK_INST(inst_)->CTLR |= CAN_CTLR_TXFP;
    else SDK_INST(inst_)->CTLR &= ~CAN_CTLR_TXFP;
}



BxCanFrame Can::receive(const CanFifoIndex fifo_idx){
    const auto & mailbox = SDK_INST(inst_)->sFIFOMailBox[std::bit_cast<uint8_t>(fifo_idx)];
    const uint32_t rxmir = mailbox.RXMIR;
    const uint32_t rxmdtr = mailbox.RXMDTR;

    const uint8_t dlc_bits = rxmdtr & (0x0F);

    const uint64_t payload_u64 = 
        static_cast<uint64_t>(mailbox.RXMDLR) 
        | (static_cast<uint64_t>(mailbox.RXMDHR) << 32);

    return BxCanFrame::from_sxx32_regs(rxmir, payload_u64, dlc_bits);
}




void Can::on_tx_interrupt(){
    // return;
    const auto temp_tstatr = SDK_INST(inst_)->TSTATR;
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
                //failed
                if(callback_ != nullptr)
                    callback_(CanEvent(CanTransmitEvent::Failed));
                SDK_INST(inst_)->TSTATR = TSTATR_RQCP_MASK;
                break;
            case TSTATR_TME_MASK | TSTATR_RQCP_MASK | TSTATR_RXOK_MASK:
                //success
                if(callback_ != nullptr)
                    callback_(CanEvent(CanTransmitEvent::Success));
                SDK_INST(inst_)->TSTATR = TSTATR_RQCP_MASK;
                break;
        }
    };

    iter_mailbox.template operator() < CanMailboxIndex::_0 > ();
    iter_mailbox.template operator() < CanMailboxIndex::_1 > ();
    iter_mailbox.template operator() < CanMailboxIndex::_2 > ();

    //poll next
    if(tx_fifo_.length() == 0) return;
    const auto may_idle_mailbox = can_get_idle_mailbox_index(inst_);
    if(may_idle_mailbox.is_none()) return;

    transmit(tx_fifo_.pop_unchecked(), may_idle_mailbox.unwrap());
}

volatile uint32_t & can_get_rfifo_reg(void * inst, CanFifoIndex fifo_idx){
    switch(fifo_idx){
        case CanFifoIndex::_0: return SDK_INST(inst)->RFIFO0;
        case CanFifoIndex::_1: return SDK_INST(inst)->RFIFO1;
    }
    __builtin_unreachable();
}

void can_clear_fmp_mask(void * inst, const CanFifoIndex fifo_idx){
    // switch(fifo_idx){
    // case CanFifoIndex::_0:
    //     SDK_INST(inst)->RFIFO0 = CAN_RFIFO0_RFOM0;
    //     break;
    // case CanFifoIndex::_1:
    //     SDK_INST(inst)->RFIFO1 = CAN_RFIFO1_RFOM1;
    //     break;
    // }
    can_get_rfifo_reg(inst, fifo_idx) = CAN_RFIFO0_RFOM0;
}


void Can::on_rx_interrupt(const CanFifoIndex fifo_idx){

    [[maybe_unused]] static constexpr uint32_t FMP_MASK = 0b00'0011;
    static constexpr uint32_t FOM_MASK = 0b10'0000;
    [[maybe_unused]] static constexpr uint32_t FFULL_MASK =  0b00'1000;
    [[maybe_unused]] static constexpr uint32_t FOV_MASK =  0b01'0000;


    [[maybe_unused]] const auto temp_inten_r = SDK_INST(inst_)->INTENR;
    auto & rfifo_reg = can_get_rfifo_reg(inst_, fifo_idx);
    const uint32_t temp_rfifo_reg = rfifo_reg;

    #if 0
    if(temp_rfifo_reg & FFULL_MASK){
        //fifo满
        {
            //TODO
            accept_rx_full_interrupt(fifo_idx);
            // __builtin_trap();
        }

        rfifo_reg = FFULL_MASK;
    }
    
    if(temp_rfifo_reg & FOV_MASK){
        ///fifo溢出
        {
            //TODO
            accept_rx_overrun_interrupt(fifo_idx);
        }
        rfifo_reg = FOV_MASK;
    }
    #endif
    
    if(temp_rfifo_reg & FOM_MASK){
        //收到can报文

        // const uint8_t num_pending = static_cast<uint8_t>((temp_rfifo_reg & FMP_MASK) >> 0);
        // for(uint8_t _ = 0; _ < num_pending; _++)

        accept_rx_frame_interrupt(fifo_idx);
        rfifo_reg = FOM_MASK;
    }
    // rfifo_reg = FFULL_MASK | FOV_MASK | FOM_MASK;

}

void Can::accept_rx_full_interrupt(const CanFifoIndex fifo_idx){
    //TODO
    __builtin_trap();
}

void Can::accept_rx_overrun_interrupt(const CanFifoIndex fifo_idx){
    //TODO
    __builtin_trap();
}

void Can::accept_rx_frame_interrupt(const CanFifoIndex fifo_idx){
    //TODO
    (void)rx_fifo_.try_push(receive(fifo_idx));
}


#ifdef CAN1_PRESENT
void USB_HP_CAN1_TX_IRQHandler(){
    can1.on_tx_interrupt();
}


#if 0
void USB_LP_CAN1_RX0_IRQHandler() {
    can1.on_rx_interrupt(CanFifoIndex::_0);
}

void CAN1_RX1_IRQHandler(){
    can1.on_rx_interrupt(CanFifoIndex::_1);
}

#else
#define CAN_RX_HANDLER(inst, uinst)\
if (MY_CAN_GetITStatus<FMP_MASK>(reg, uinst)){\
    inst.accept_rx_frame_interrupt(FIFO_IDX);\
    can_clear_fmp_mask(uinst, FIFO_IDX);\
}\
if(MY_CAN_GetITStatus<FF_MASK>(reg, uinst)){\
    inst.accept_rx_full_interrupt(FIFO_IDX);\
    can_clear_it_pending_bit<FF_MASK>(uinst);\
}\
if(MY_CAN_GetITStatus<FOV_MASK>(reg, uinst)){\
    inst.accept_rx_overrun_interrupt(FIFO_IDX);\
    can_clear_it_pending_bit<FOV_MASK>(uinst);\
}\


void USB_LP_CAN1_RX0_IRQHandler() {
    static constexpr auto FMP_MASK = CAN_IT_FMP0;
    static constexpr auto FF_MASK = CAN_IT_FF0;
    static constexpr auto FOV_MASK = CAN_IT_FOV0;
    static constexpr auto FIFO_IDX = CanFifoIndex::_0;
    const auto reg = CAN1->INTENR;
    CAN_RX_HANDLER(can1, CAN1)
}

void CAN1_RX1_IRQHandler(){
    static constexpr auto FMP_MASK = CAN_IT_FMP1;
    static constexpr auto FF_MASK = CAN_IT_FF1;
    static constexpr auto FOV_MASK = CAN_IT_FOV1;
    static constexpr auto FIFO_IDX = CanFifoIndex::_1;
    const auto reg = CAN1->INTENR;
    CAN_RX_HANDLER(can1, CAN1)
}
#endif

#ifdef CAN_SCE_ENABLED
void CAN1_SCE_IRQHandler(){
    can1.on_sce_interrupt();
}
#endif
#endif

#ifdef CAN2_PRESENT
void CAN2_TX_IRQHandler(){
    can2.on_tx_interrupt();
}

void CAN2_RX0_IRQHandler(){
    can1.on_rx_interrupt(CanFifoIndex::_0);
}

void CAN2_RX1_IRQHandler(){
    can1.on_rx_interrupt(CanFifoIndex::_1);
}

#ifdef CAN_SCE_ENABLED
void CAN2_SCE_IRQHandler(){
    can2.on_sce_interrupt();
}

#endif
#endif

namespace ymd::hal{

#ifdef CAN1_PRESENT
Can can1 = Can{CAN1};
#endif

#ifdef CAN2_PRESENT
Can can2 = Can{CAN2};
#endif
}


void Can::on_sce_interrupt(){
    #if 1
    const auto reg = SDK_INST(inst_)->INTENR;
    // #ifdef CAN_SCE_ENABLED
    if (MY_CAN_GetITStatus<CAN_IT_WKU>(reg, SDK_INST(inst_))) {
        // Handle Wake-up interrupt
        can_clear_it_pending_bit<CAN_IT_WKU>(SDK_INST(inst_));
    } else if (MY_CAN_GetITStatus<CAN_IT_SLK>(reg, SDK_INST(inst_))) {
        // Handle Sleep acknowledge interrupt
        can_clear_it_pending_bit<CAN_IT_SLK>(SDK_INST(inst_));
    } else if (MY_CAN_GetITStatus<CAN_IT_ERR>(reg, SDK_INST(inst_))) {
        // Handle Error interrupt
        can_clear_it_pending_bit<CAN_IT_ERR>(SDK_INST(inst_));
    } else if (MY_CAN_GetITStatus<CAN_IT_EWG>(reg, SDK_INST(inst_))) {
        // Handle Error warning interrupt
        can_clear_it_pending_bit<CAN_IT_EWG>(SDK_INST(inst_));
    } else if (MY_CAN_GetITStatus<CAN_IT_EPV>(reg, SDK_INST(inst_))) {
        // Handle Error passive interrupt
        can_clear_it_pending_bit<CAN_IT_EPV>(SDK_INST(inst_));
    } else if (MY_CAN_GetITStatus<CAN_IT_BOF>(reg, SDK_INST(inst_))) {
        // Handle Bus-off interrupt
        can_clear_it_pending_bit<CAN_IT_BOF>(SDK_INST(inst_));
    } else if (MY_CAN_GetITStatus<CAN_IT_LEC>(reg, SDK_INST(inst_))) {
        // Handle Last error code interrupt
        can_clear_it_pending_bit<CAN_IT_LEC>(SDK_INST(inst_));
    } else {
        // Handle other interrupts or add more cases as needed
    }
    #endif
}
