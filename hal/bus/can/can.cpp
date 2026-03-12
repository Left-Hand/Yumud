#include "ral/can.hpp"

#include "core/debug/debug.hpp"
#include "core/utils/scope_guard.hpp"
#include "core/sdk.hpp"

#include "can.hpp"
#include "can_layout.hpp"

#include "hal/sysmisc/nvic/nvic.hpp"
#include "hal/gpio/gpio_port.hpp"


using namespace ymd;
using namespace ymd::hal;


//CAN发送中断 NVIC优先级
static constexpr NvicPriority CAN_TX_INTERRUPT_NVIC_PRIORITY = {1, 4};

//CAN接收中断0 NVIC优先级
static constexpr NvicPriority CAN_RX0_INTERRUPT_NVIC_PRIORITY = {1, 2};

//CAN接收中断1 NVIC优先级
static constexpr NvicPriority CAN_RX1_INTERRUPT_NVIC_PRIORITY = {1, 2};

//CAN状态改变中断 NVIC优先级
static constexpr NvicPriority CAN_SCE_INTERRUPT_NVIC_PRIORITY = {1, 1};






using EventCallback = Can::EventCallback;


#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SPL_INST(x) (reinterpret_cast<COPY_CONST(x, CAN_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::CAN_Def)>(x))


#define CAN_READABLE_ABORT_REASON_EN 0

#ifdef RELEASE

//release 模式下尽可能避免panic导致的用户体验下降 同时对错误进行沉默处理 优化性能

#define DEBUG_TRAP() {};
#define DEBUG_UNREACHABLE() __builtin_unreachable();

#define TRY_EMIT_EVENT(self, x)   \
if(self.event_callback_ != nullptr) {\
    self.event_callback_(x);\
}else{/* do nothing */}

#define TRY_EMIT_EVENT_OR_ABORT(self, x, str) TRY_EMIT_EVENT(self, x);
#define ABORT(str) {;};


#else


#define DEBUG_TRAP() __builtin_trap();
#define DEBUG_UNREACHABLE() __builtin_trap();


#define TRY_EMIT_EVENT(self, x)   \
if(self.event_callback_ != nullptr) {\
    self.event_callback_(x);\
}\

#if CAN_READABLE_ABORT_REASON_EN == 1

    #define TRY_EMIT_EVENT_OR_ABORT(self, x, str)\
    if(self.event_callback_ != nullptr){\
        self.event_callback_(x);\
    } else{\
        sys::abort(AbortInfo::from_reason(str));\
    }\

    #define ABORT(str)\
    sys::abort(AbortInfo::from_reason(str));\

#else

    #define TRY_EMIT_EVENT_OR_ABORT(self, x, str)\
    if(self.event_callback_ != nullptr){\
        self.event_callback_(x);\
    } else{\
        sys::abort(AbortInfo::from_default());\
    }\

    #define ABORT(str)\
    sys::abort(AbortInfo::from_default());\


#endif
#endif


namespace {


template<CanRemap REMAP>
[[maybe_unused]] static Gpio _can_to_tx_pin(const Nth nth){
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
    DEBUG_UNREACHABLE();
}

template<CanRemap REMAP>
[[maybe_unused]] static Gpio _can_to_rx_pin(const Nth nth){
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
    DEBUG_UNREACHABLE();
}

#define DEF_CAN_BIND_PIN_LAYOUTER(name)\
[[maybe_unused]] static Gpio can_to_##name##_pin(const Nth nth, const CanRemap remap){\
    switch(remap){\
        case CanRemap::_0: return _can_to_##name##_pin<CanRemap::_0>(nth);\
        case CanRemap::_1: return _can_to_##name##_pin<CanRemap::_1>(nth);\
        case CanRemap::_2: return _can_to_##name##_pin<CanRemap::_2>(nth);\
        case CanRemap::_3: return _can_to_##name##_pin<CanRemap::_3>(nth);\
    }\
    DEBUG_UNREACHABLE();\
}\

DEF_CAN_BIND_PIN_LAYOUTER(tx)
DEF_CAN_BIND_PIN_LAYOUTER(rx)

#undef DEF_CAN_BIND_PIN_LAYOUTER
}


namespace {


static Option<CanMailboxIndex> can_get_idle_mailbox_index(void * p_inst){
    static constexpr uint32_t ANY_MAILBOX_IDLE_BITMASK = 
        (lld::can_tstatr_tme_mask(CanMailboxIndex::_0) 
        | lld::can_tstatr_tme_mask(CanMailboxIndex::_1) 
        | lld::can_tstatr_tme_mask(CanMailboxIndex::_2));

    const uint32_t tempreg = SPL_INST(p_inst)->TSTATR;
    const bool is_any_mailbox_idle = (tempreg & ANY_MAILBOX_IDLE_BITMASK) != 0;
    if(not is_any_mailbox_idle) return None;
    const uint8_t idle_mbox_idx_bits = static_cast<uint8_t>((tempreg & CAN_TSTATR_CODE) >> 24);
    return Some(std::bit_cast<CanMailboxIndex>(idle_mbox_idx_bits));
};

template<CanFifoIndex fifo_idx>
[[nodiscard]] static volatile uint32_t & can_get_rfifo_reg(void * p_inst){
    switch(fifo_idx){
        case CanFifoIndex::_0: return SPL_INST(p_inst)->RFIFO0;
        case CanFifoIndex::_1: return SPL_INST(p_inst)->RFIFO1;
    }
    __builtin_unreachable();
}

template<uint32_t IT>
[[nodiscard]] static ITStatus can_get_it_status(void * p_inst, const uint32_t inten_reg){
    if((inten_reg & IT) != RESET){
        if constexpr(IT == CAN_IT_TME){
            return(SPL_INST(p_inst)->TSTATR & (CAN_TSTATR_RQCP0|CAN_TSTATR_RQCP1|CAN_TSTATR_RQCP2));  
        }else if constexpr(IT == CAN_IT_FMP0){
            return (SPL_INST(p_inst)->RFIFO0 & CAN_RFIFO0_FMP0);  
        }else if constexpr(IT == CAN_IT_FF0){
            return (SPL_INST(p_inst)->RFIFO0 & CAN_RFIFO0_FULL0);  
        }else if constexpr(IT == CAN_IT_FOV0){
            return (SPL_INST(p_inst)->RFIFO0 & CAN_RFIFO0_FOVR0);  
        }else if constexpr(IT == CAN_IT_FMP1){
            return (SPL_INST(p_inst)->RFIFO1 & CAN_RFIFO1_FMP1);  
        }else if constexpr(IT == CAN_IT_FF1){
            return (SPL_INST(p_inst)->RFIFO1 & CAN_RFIFO1_FULL1);  
        }else if constexpr(IT == CAN_IT_FOV1){
            return (SPL_INST(p_inst)->RFIFO1 & CAN_RFIFO1_FOVR1);  
        }else if constexpr (IT == CAN_IT_WKU)
            return (SPL_INST(p_inst)->STATR & CAN_STATR_WKUI);  
        else if constexpr (IT == CAN_IT_SLK)
            return (SPL_INST(p_inst)->STATR & CAN_STATR_SLAKI);  
        else if constexpr (IT == CAN_IT_EWG)
            return (SPL_INST(p_inst)->ERRSR & CAN_ERRSR_EWGF);  
        else if constexpr (IT == CAN_IT_EPV)
            return (SPL_INST(p_inst)->ERRSR & CAN_ERRSR_EPVF); 
        else if constexpr (IT == CAN_IT_BOF)
            return (SPL_INST(p_inst)->ERRSR & CAN_ERRSR_BOFF);  
        else if constexpr (IT == CAN_IT_LEC)
            return (SPL_INST(p_inst)->ERRSR & CAN_ERRSR_LEC);  
        else if constexpr (IT == CAN_IT_ERR)
            return (SPL_INST(p_inst)->STATR & CAN_STATR_ERRI); 
    }
    return RESET;
}


template<uint32_t IT>
static void can_clear_it_pending_bit(void * p_inst)
{
    if constexpr(IT == CAN_IT_TME){
        SPL_INST(p_inst)->TSTATR = CAN_TSTATR_RQCP0 | CAN_TSTATR_RQCP1 | CAN_TSTATR_RQCP2;  
    }else if constexpr(IT == CAN_IT_FF0){
        SPL_INST(p_inst)->RFIFO0 = CAN_RFIFO0_FULL0;
    }else if constexpr(IT == CAN_IT_FOV0){
        SPL_INST(p_inst)->RFIFO0 = CAN_RFIFO0_FOVR0;
    }else if constexpr(IT == CAN_IT_FF1){
        SPL_INST(p_inst)->RFIFO1 = CAN_RFIFO1_FULL1;
    }else if constexpr(IT == CAN_IT_FOV1){
        SPL_INST(p_inst)->RFIFO1 = CAN_RFIFO1_FOVR1;
    }else if constexpr(IT == CAN_IT_WKU){
        SPL_INST(p_inst)->STATR = CAN_STATR_WKUI;
    }else if constexpr(IT == CAN_IT_SLK){
        SPL_INST(p_inst)->STATR = CAN_STATR_SLAKI;
    }else if constexpr(IT == CAN_IT_EWG){
        SPL_INST(p_inst)->STATR = CAN_STATR_ERRI;
    }else if constexpr(IT == CAN_IT_EPV){
        SPL_INST(p_inst)->STATR = CAN_STATR_ERRI; 
    }else if constexpr (IT == CAN_IT_BOF){
        SPL_INST(p_inst)->STATR = CAN_STATR_ERRI; 
    }else if constexpr (IT == CAN_IT_LEC){
        SPL_INST(p_inst)->ERRSR = RESET; 
        SPL_INST(p_inst)->STATR = CAN_STATR_ERRI; 
    }else if constexpr (IT == CAN_IT_ERR){
        SPL_INST(p_inst)->ERRSR = RESET; 
        SPL_INST(p_inst)->STATR = CAN_STATR_ERRI; 
	}
}


static void can_setup_interrupts(void * p_inst){
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

    CAN_ClearITPendingBit(SPL_INST(p_inst), it_mask);
    CAN_ITConfig(SPL_INST(p_inst), it_mask, ENABLE);

    const auto inst_nth = lld::can_to_nth(reinterpret_cast<uintptr_t>(p_inst));
    switch(inst_nth.count()){
        #ifdef CAN1_PRESENT
        case 1:
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
        case 2:
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
        case 3:
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
            DEBUG_TRAP();
            break;
    }
}



static constexpr uint32_t CAN_RFIFO_FMP_MASK = 0b00'0011;
static constexpr uint32_t CAN_RFIFO_FOM_MASK = 0b10'0000;
static constexpr uint32_t CAN_RFIFO_FFULL_MASK =  0b00'1000;
static constexpr uint32_t CAN_RFIFO_FOV_MASK =  0b01'0000;

}

Can::Can(void * p_inst):
    p_inst_(p_inst),
    inst_nth_(lld::can_to_nth(reinterpret_cast<uintptr_t>(p_inst)))
    {;}


void Can::init(const Config & cfg){
    tx_queue_.clear();
    rx_queue_.clear();

    set_remap(cfg.remap);
    alter_to_pins(cfg.remap);

    lld::can_enable_rcc(inst_nth_, EN);

    const auto bit_timming_coeffs = [&] -> CanNominalBitTimmingCoeffs{
        const auto & bit_timming = cfg.bit_timming;
        if(bit_timming.is<CanBaudrate>()){
            return (bit_timming.unwrap_as<CanBaudrate>()).to_coeffs(get_aligned_bus_clk_freq());
        }else if(bit_timming.is<CanNominalBitTimmingCoeffs>()){
            return bit_timming.unwrap_as<CanNominalBitTimmingCoeffs>();
        }else{
            //运行到这里说明库组件一致性出现问题 请联系我修复
            DEBUG_UNREACHABLE();
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

    if(const auto status = lld::my_barecan_init(SPL_INST(p_inst_), &CAN_InitConf);
        status == CAN_InitStatus_Failed){
        //初始化失败
        DEBUG_TRAP();
    }


    can_setup_interrupts(p_inst_);

}


Result<void, Infallible> Can::configure_filter(
    const Nth filter_nth, 
    const hal::CanFifoIndex route_fifo_idx,
    const hal::CanFilterConfig & filter_cfg
){ 
    if(filter_nth.count() >= lld::NUM_CAN_FILTERS) ABORT("filter_nth out of range");
    lld::can_configure_filter(filter_nth.count(), route_fifo_idx, filter_cfg);
    return Ok();
}



Result<void, Infallible> Can::set_filter_origin(
    const Nth filter_offset 
){
    if(inst_nth_.count() == 1) {
        if(filter_offset.count() != 0){
            ABORT("can1 filter origin is always 0");
        }
    }
    
    lld::can_set_filter_origin(inst_nth_.count(), filter_offset.count());
    return Ok();
}

void Can::deinit(){
    enable_rcc(DISEN);
};

void Can::init_interrupts(){
    can_setup_interrupts(p_inst_);
}

void Can::transmit(CanMailboxIndex mbox_idx, const BxCanFrame & frame){
    lld::can_transmit(p_inst_, mbox_idx, frame);
}

Result<void, CanLibError> Can::try_write(const BxCanFrame & frame){
    //查找空闲的邮箱
    const auto may_idle_mbox_idx = can_get_idle_mailbox_index(p_inst_);

    //如果有空闲邮箱，直接发送
    if(may_idle_mbox_idx.is_some()){
        const auto idle_mbox_idx = may_idle_mbox_idx.unwrap();
        transmit(idle_mbox_idx, frame);
        return Ok();
    }
    
    //没有空闲邮箱，存入队列
    if(const auto write_len = tx_queue_.try_push(frame);
        write_len == 0){
        // 队列已满
        return Err(CanLibError::SoftQueueFull);
    }
    return Ok();
}

Option<BxCanFrame> Can::try_read(){
    Option<BxCanFrame> ret = None;

    if(const auto quantity = rx_queue_.consume_one([&](const BxCanFrame & frame){
        ret = Some(frame);
    }); quantity == 0) return None;
    return ret;
}


BxCanFrame Can::receive(const CanFifoIndex fifo_idx){
    return lld::can_receive(p_inst_, fifo_idx);
}


size_t Can::free_capacity(){
    return tx_queue_.free_capacity();
}

size_t Can::available(){
    return rx_queue_.length();
}

void Can::alter_to_pins(const CanRemap remap){
    can_to_tx_pin(inst_nth_, remap).afpp();
    can_to_rx_pin(inst_nth_, remap).afpp();
}



void Can::enable_rcc(const Enable en){
    lld::can_enable_rcc(inst_nth_, en);
}

void Can::set_remap(const CanRemap remap){
    lld::can_set_remap(inst_nth_, remap);
}


void Can::enable_hw_retransmit(const Enable en){
    RAL_INST(p_inst_)->CTLR.NART = (en == EN);
}

uint32_t Can::get_aligned_bus_clk_freq(){
    //所有的CAN外设都使用APB1时钟
    return sys::clock::get_apb1_clk_freq();
}

uint8_t Can::get_rx_errcnt(){
    return static_cast<uint8_t>(SPL_INST(p_inst_)->ERRSR >> 24);
}

uint8_t Can::get_tx_errcnt(){
    return static_cast<uint8_t>(SPL_INST(p_inst_)->ERRSR >> 16);
}

Option<Can::Error> Can::last_error(){
    const uint8_t bits = static_cast<uint8_t>(RAL_INST(p_inst_)->ERRSR.LEC & 0xff);
    if(bits == 0) return None;
    return Some(std::bit_cast<Can::Error>(bits));
}

bool Can::is_tranmitting(){
    return bool(RAL_INST(p_inst_)->STATR.TXM);
}

bool Can::is_receiving(){
    return bool(RAL_INST(p_inst_)->STATR.RXM);
}

bool Can::is_busoff(){
    return bool(RAL_INST(p_inst_)->ERRSR.BOFF);
}



void Can::abort_transmit(const CanMailboxIndex mbox_idx){
    SPL_INST(p_inst_)->TSTATR = lld::can_statr_rqcp_mask(mbox_idx);
}

void Can::abort_all_transmits(){
    static constexpr uint32_t MASK = 
        lld::can_statr_rqcp_mask(CanMailboxIndex::_0)  
        | lld::can_statr_rqcp_mask(CanMailboxIndex::_1)  
        | lld::can_statr_rqcp_mask(CanMailboxIndex::_2)
    ;
    SPL_INST(p_inst_)->TSTATR = MASK;
}

void Can::enable_rxfifo_lock(const Enable en){
    RAL_INST(p_inst_)->CTLR.RFLM = (en == EN);
}

void Can::enable_index_priority(const Enable en){
    RAL_INST(p_inst_)->CTLR.TXFP = (en == EN);
}

void Can::enable_debug_freeze(const Enable en){
    RAL_INST(p_inst_)->CTLR.DBF = (en == EN);
}


void CanInterruptDispatcher::isr_tx(Can & self){
    volatile uint32_t & tstatr_reg = SPL_INST(self.p_inst_)->TSTATR;
    const auto temp_tstatr = tstatr_reg;
    //遍历每个邮箱

    auto iter_mailbox = [&]<CanMailboxIndex mbox_idx>() __attribute__((always_inline)){
        static constexpr uint32_t TSTATR_TME_MASK = lld::can_tstatr_tme_mask(mbox_idx);
        static constexpr uint32_t TSTATR_RQCP_MASK = lld::can_statr_rqcp_mask(mbox_idx);
        static constexpr uint32_t TSTATR_TXOK_MASK = lld::can_statr_tkok_mask(mbox_idx);
        static constexpr uint32_t READY_MASK = TSTATR_TME_MASK | TSTATR_RQCP_MASK;

        if((temp_tstatr & READY_MASK) != READY_MASK){
            //not ready
            return;
        }

        auto isr_clear_guard = make_scope_guard([&]{
            //清除发送标志位
            tstatr_reg = TSTATR_RQCP_MASK;
        });

        const bool is_success = (temp_tstatr & TSTATR_TXOK_MASK) != 0;
        const CanTransmitEvent::Kind ev_kind = is_success ? 
            CanTransmitEvent::Kind::Success : CanTransmitEvent::Kind::Failed;

        const auto tx_ev = hal::CanTransmitEvent{
            .kind = ev_kind,
            .mbox_idx = mbox_idx
        };
        const auto ev = CanEvent::from(tx_ev);
        TRY_EMIT_EVENT(self, ev);
    };

    iter_mailbox.template operator() < CanMailboxIndex::_0 > ();
    iter_mailbox.template operator() < CanMailboxIndex::_1 > ();
    iter_mailbox.template operator() < CanMailboxIndex::_2 > ();


    self.poll_tx_queue();
}

void Can::poll_tx_queue(){
    auto & self = *this;
    if(self.tx_queue_.length() == 0) return;
    const auto may_idle_mailbox = can_get_idle_mailbox_index(self.p_inst_);
    if(may_idle_mailbox.is_none()) return;

    if(const auto quantity = tx_queue_.consume_one([&](const hal::BxCanFrame & frame){
        self.transmit(may_idle_mailbox.unwrap(), frame);
    }); quantity == 0){
        //can't consume
        // this shouldn't never happen,but silently ignore it
        return;
    }
    return;
}

void CanInterruptDispatcher::isr_rx0(Can & can){
    CanInterruptDispatcher::isr_rx(
        can, 
        can_get_rfifo_reg<CanFifoIndex::_0>(can.p_inst_), 
        CanFifoIndex::_0
    );
}

void CanInterruptDispatcher::isr_rx1(Can & can){
    CanInterruptDispatcher::isr_rx(
        can, 
        can_get_rfifo_reg<CanFifoIndex::_1>(can.p_inst_), 
        CanFifoIndex::_1
    );
}

void CanInterruptDispatcher::isr_rx(
    Can & self, 
    volatile uint32_t & rfifo_reg, 
    const CanFifoIndex fifo_idx
){
    const uint32_t temp_rfifo_reg = rfifo_reg;

    if(temp_rfifo_reg & CAN_RFIFO_FFULL_MASK){
        auto isr_clear_guard = make_scope_guard([&](){
            rfifo_reg = CAN_RFIFO_FFULL_MASK;
        });

        //rfifo满
        {
            const auto rx_ev = hal::CanReceiveEvent{
                .kind = CanReceiveEvent::Kind::FifoFull,
                .fifo_idx = fifo_idx
            };
            const auto ev = CanEvent::from(rx_ev);
            TRY_EMIT_EVENT(self, ev)
        }
    }
    
    if(temp_rfifo_reg & CAN_RFIFO_FOV_MASK){
        auto isr_clear_guard = make_scope_guard([&](){
            rfifo_reg = CAN_RFIFO_FOV_MASK;
        });

        ///rfifo溢出
        {
            const auto rx_ev = hal::CanReceiveEvent{
                .kind = CanReceiveEvent::Kind::FifoOverrun,
                .fifo_idx = fifo_idx
            };
            const auto ev = CanEvent::from(rx_ev);
            TRY_EMIT_EVENT_OR_ABORT(self, ev, "can rxfifo overrun")
        }
    }

    //注意这里是判断fmp掩码 但是清零的是fom
    if (temp_rfifo_reg & CAN_RFIFO_FMP_MASK){
        auto isr_clear_guard = make_scope_guard([&](){
            rfifo_reg = CAN_RFIFO_FOM_MASK;
        });

        //收到新的报文
        {
            if(const auto quantity = self.rx_queue_.try_push(self.receive(fifo_idx));
                quantity == 0) return;
        }

        {
            const auto rx_ev = hal::CanReceiveEvent{
                .kind = CanReceiveEvent::Kind::FifoPending,
                .fifo_idx = fifo_idx
            };
            const auto ev = CanEvent::from(rx_ev);
            TRY_EMIT_EVENT(self, ev)
        }
    }
}

void CanInterruptDispatcher::isr_sce(Can & self){
    void * p_inst = self.p_inst_;
    const uint32_t temp_inten_reg = SPL_INST(p_inst)->INTENR;

    auto flag_bits = hal::CanStatusFlag::zero();

    if (can_get_it_status<CAN_IT_WKU>(SPL_INST(p_inst), temp_inten_reg)) {
        // Handle Wake-up interrupt
        // 唤醒中断
        flag_bits.wakeup = 1;
        can_clear_it_pending_bit<CAN_IT_WKU>(SPL_INST(p_inst));
    } 
    
    if (can_get_it_status<CAN_IT_SLK>(SPL_INST(p_inst), temp_inten_reg)) {
        // Handle Sleep acknowledge interrupt
        // 睡眠确认中断
        flag_bits.sleep_acknowledge = 1;
        can_clear_it_pending_bit<CAN_IT_SLK>(SPL_INST(p_inst));
    } 
    
    if (can_get_it_status<CAN_IT_ERR>(SPL_INST(p_inst), temp_inten_reg)) {
        // Handle Error interrupt
        // 错误中断
        flag_bits.error = 1;
        can_clear_it_pending_bit<CAN_IT_ERR>(SPL_INST(p_inst));
    } 
    
    if (can_get_it_status<CAN_IT_EWG>(SPL_INST(p_inst), temp_inten_reg)) {
        // Handle Error warning interrupt
        // 主动错误中断
        flag_bits.error_warning = 1;
        can_clear_it_pending_bit<CAN_IT_EWG>(SPL_INST(p_inst));
    } 
    
    if (can_get_it_status<CAN_IT_EPV>(SPL_INST(p_inst), temp_inten_reg)) {
        // Handle Error passive interrupt
        // 被动错误中断
        flag_bits.error_passive = 1;
        can_clear_it_pending_bit<CAN_IT_EPV>(SPL_INST(p_inst));
    } 
    
    if (can_get_it_status<CAN_IT_BOF>(SPL_INST(p_inst), temp_inten_reg)) {
        // Handle Bus-off interrupt
        // 掉线中断
        flag_bits.bus_off = 1;
        can_clear_it_pending_bit<CAN_IT_BOF>(SPL_INST(p_inst));
    } 
    
    if (can_get_it_status<CAN_IT_LEC>(SPL_INST(p_inst), temp_inten_reg)) {
        // Handle Last error code interrupt
        // 错误码中断
        flag_bits.last_error_code = 1;
        can_clear_it_pending_bit<CAN_IT_LEC>(SPL_INST(p_inst));
    }

    TRY_EMIT_EVENT(self, CanEvent::from(flag_bits));
}

