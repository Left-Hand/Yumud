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
static constexpr NvicPriorityCode CAN_TX_INTERRUPT_NVIC_PRIORITY = 
    NvicPriorityCode::from_pre_sub_dn<2>(1, 3);

//CAN接收中断0 NVIC优先级
static constexpr NvicPriorityCode CAN_RX0_INTERRUPT_NVIC_PRIORITY = 
    NvicPriorityCode::from_pre_sub_dn<2>(1, 2);

//CAN接收中断1 NVIC优先级
static constexpr NvicPriorityCode CAN_RX1_INTERRUPT_NVIC_PRIORITY = 
    NvicPriorityCode::from_pre_sub_dn<2>(1, 0);

//CAN状态改变中断 NVIC优先级
static constexpr NvicPriorityCode CAN_SCE_INTERRUPT_NVIC_PRIORITY = 
    NvicPriorityCode::from_pre_sub_dn<2>(1, 1);


static constexpr auto CAN1_TX_INTERRUPT_NVIC_PRIORITY = CAN_TX_INTERRUPT_NVIC_PRIORITY;
static constexpr auto CAN1_RX0_INTERRUPT_NVIC_PRIORITY = CAN_RX0_INTERRUPT_NVIC_PRIORITY;
static constexpr auto CAN1_RX1_INTERRUPT_NVIC_PRIORITY = CAN_RX1_INTERRUPT_NVIC_PRIORITY;
static constexpr auto CAN1_SCE_INTERRUPT_NVIC_PRIORITY = CAN_SCE_INTERRUPT_NVIC_PRIORITY;

static constexpr auto CAN2_TX_INTERRUPT_NVIC_PRIORITY = CAN_TX_INTERRUPT_NVIC_PRIORITY;
static constexpr auto CAN2_RX0_INTERRUPT_NVIC_PRIORITY = CAN_RX0_INTERRUPT_NVIC_PRIORITY;
static constexpr auto CAN2_RX1_INTERRUPT_NVIC_PRIORITY = CAN_RX1_INTERRUPT_NVIC_PRIORITY;
static constexpr auto CAN2_SCE_INTERRUPT_NVIC_PRIORITY = CAN_SCE_INTERRUPT_NVIC_PRIORITY;

static constexpr auto CAN3_TX_INTERRUPT_NVIC_PRIORITY = CAN_TX_INTERRUPT_NVIC_PRIORITY;
static constexpr auto CAN3_RX0_INTERRUPT_NVIC_PRIORITY = CAN_RX0_INTERRUPT_NVIC_PRIORITY;
static constexpr auto CAN3_RX1_INTERRUPT_NVIC_PRIORITY = CAN_RX1_INTERRUPT_NVIC_PRIORITY;
static constexpr auto CAN3_SCE_INTERRUPT_NVIC_PRIORITY = CAN_SCE_INTERRUPT_NVIC_PRIORITY;



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


[[maybe_unused]] [[deprecated("可以参考学习 但是这个函数不再被调用")]] 
static Option<CanMailboxIndex> can_get_idle_mailbox_index(void * p_inst){
    static constexpr uint32_t ANY_MAILBOX_IDLE_BITMASK = 
        (lld::can_tstatr_tme_mask(CanMailboxIndex::_0) 
        | lld::can_tstatr_tme_mask(CanMailboxIndex::_1) 
        | lld::can_tstatr_tme_mask(CanMailboxIndex::_2));

    const uint32_t temp_tstar = SPL_INST(p_inst)->TSTATR;
    const bool is_any_mailbox_idle = (temp_tstar & ANY_MAILBOX_IDLE_BITMASK) != 0;
    if(not is_any_mailbox_idle) return None;
    const uint8_t idle_mbox_idx_bits = static_cast<uint8_t>(((temp_tstar) >> 24) & 0b11);
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
            lld::nvic_set_irqn_priority(USB_HP_CAN1_TX_IRQn, CAN1_TX_INTERRUPT_NVIC_PRIORITY);
            lld::nvic_enable_irqn(USB_HP_CAN1_TX_IRQn, true);
            //rx0 interrupt
            lld::nvic_set_irqn_priority(USB_LP_CAN1_RX0_IRQn, CAN1_RX0_INTERRUPT_NVIC_PRIORITY);
            lld::nvic_enable_irqn(USB_LP_CAN1_RX0_IRQn, true);
            //rx1 interrupt
            lld::nvic_set_irqn_priority(CAN1_RX1_IRQn, CAN1_RX1_INTERRUPT_NVIC_PRIORITY);
            lld::nvic_enable_irqn(CAN1_RX1_IRQn, true);
            //sce interrupt

            #ifdef CAN_SCE_ENABLED
            lld::nvic_set_irqn_priority(CAN1_SCE_IRQn, CAN1_SCE_INTERRUPT_NVIC_PRIORITY);
            lld::nvic_enable_irqn(CAN1_SCE_IRQn, true);
            #endif
            break;
        #endif

        #ifdef CAN2_PRESENT
        case 2:

            //tx interrupt
            lld::nvic_set_irqn_priority(CAN2_TX_IRQn, CAN2_TX_INTERRUPT_NVIC_PRIORITY);
            lld::nvic_enable_irqn(CAN2_TX_IRQn, true);
            //rx0 interrupt
            lld::nvic_set_irqn_priority(CAN2_RX0_IRQn, CAN2_RX0_INTERRUPT_NVIC_PRIORITY);
            lld::nvic_enable_irqn(CAN2_RX0_IRQn, true);
            //rx1 interrupt
            lld::nvic_set_irqn_priority(CAN2_RX1_IRQn, CAN2_RX1_INTERRUPT_NVIC_PRIORITY);
            lld::nvic_enable_irqn(CAN2_RX1_IRQn, true);
            //sce interrupt

            #ifdef CAN_SCE_ENABLED
            lld::nvic_set_irqn_priority(CAN2_SCE_IRQn, CAN2_SCE_INTERRUPT_NVIC_PRIORITY);
            lld::nvic_enable_irqn(CAN2_SCE_IRQn, true);
            #endif
            break;
        #endif

        #ifdef CAN3_PRESENT
        case 3:
            //tx interrupt
            lld::nvic_set_irqn_priority(CAN3_TX_IRQn, CAN3_TX_INTERRUPT_NVIC_PRIORITY);
            lld::nvic_enable_irqn(CAN3_TX_IRQn, true);
            //rx0 interrupt
            lld::nvic_set_irqn_priority(CAN3_RX0_IRQn, CAN3_RX0_INTERRUPT_NVIC_PRIORITY);
            lld::nvic_enable_irqn(CAN3_RX0_IRQn, true);
            //rx1 interrupt
            lld::nvic_set_irqn_priority(CAN3_RX1_IRQn, CAN3_RX1_INTERRUPT_NVIC_PRIORITY);
            lld::nvic_enable_irqn(CAN3_RX1_IRQn, true);
            //sce interrupt

            #ifdef CAN_SCE_ENABLED
            lld::nvic_set_irqn_priority(CAN3_SCE_IRQn, CAN3_SCE_INTERRUPT_NVIC_PRIORITY);
            lld::nvic_enable_irqn(CAN3_SCE_IRQn, true);
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
    // ASSERT(rx_queue_.length() == 0);

    set_remap(cfg.remap);
    alter_to_pins(cfg.remap);

    #if 0
    //不要启用这句 否则会直接变砖
    //破解方式 1.poweroff erase 此时会失败 2.然后给单片机手动复位 3.再次power erase就会成功
    //原因待后续查明
    lld::can_reset(p_inst_);
    #endif
    lld::can_enable_rcc(inst_nth_, EN);

    const auto bit_timming_coeffs = [&] -> CanNominalBitTimmingCoeffs{
        const auto & bit_timming = cfg.bit_timming;
        if(bit_timming.is<CanBaudrate>()){
            const auto coeffs = ({
                const auto may_coeffs = (bit_timming.unwrap_as<CanBaudrate>())
                    .try_into_coeffs(get_aligned_bus_clk_freq());
                if(may_coeffs.is_none()) ABORT("can't parse baudrate");
                may_coeffs.unwrap();
            });

            return coeffs;
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
        .CAN_SJW = bit_timming_coeffs.swj.tq.to_bits(),
        .CAN_BS1 = bit_timming_coeffs.bs1.tq.to_bits(),
        .CAN_BS2 = bit_timming_coeffs.bs2.tq.to_bits(),


        .CAN_TTCM = DISABLE,
        .CAN_ABOM = ENABLE,
        .CAN_AWUM = DISABLE,
        .CAN_NART = DISABLE,
        .CAN_RFLM = DISABLE,
        .CAN_TXFP = DISABLE,
    };

    if(const auto res = lld::can_initialze(p_inst_, &CAN_InitConf);
        res.is_err()){
        //初始化失败
        DEBUG_TRAP();
    }


    can_setup_interrupts(p_inst_);

}


void Can::deinit(){
    lld::can_deinit(inst_nth_);
};

void Can::init_interrupts(){
    can_setup_interrupts(p_inst_);
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

bool Can::is_tranmitting(){
    return bool(RAL_INST(p_inst_)->STATR.TXM);
}

bool Can::is_receiving(){
    return bool(RAL_INST(p_inst_)->STATR.RXM);
}

bool Can::is_sleeping(){
    return bool(RAL_INST(p_inst_)->STATR.SLAK);
}

bool Can::is_initializing(){
    return bool(RAL_INST(p_inst_)->STATR.INAK);
}

bool Can::is_busoff(){
    return bool(RAL_INST(p_inst_)->ERRSR.BOFF);
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


void Can::abort_all_transmits(){
    static constexpr uint32_t MASK = 
        lld::can_statr_abrq_mask(CanMailboxIndex::_0)  
        | lld::can_statr_abrq_mask(CanMailboxIndex::_1)  
        | lld::can_statr_abrq_mask(CanMailboxIndex::_2)
    ;
    SPL_INST(p_inst_)->TSTATR = MASK;
}

void Can::abort_transmit(const CanMailboxIndex mbox_idx){
    SPL_INST(p_inst_)->TSTATR = lld::can_statr_abrq_mask(mbox_idx);
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

uint32_t can_get_aligned_bus_clk_freq(){
    #if defined(CH32V203) || defined(CH32V303) || defined(CH32L103)
    //所有的CAN外设都使用APB1时钟
    return sys::clock::get_apb1_clk_freq();
    #elif defined(CH32H417)
    return sys::clock::get_ahb_clk_freq();
    #endif
}


uint32_t Can::get_aligned_bus_clk_freq(){
    return can_get_aligned_bus_clk_freq();
}

size_t Can::free_capacity(){
    return tx_queue_.free_capacity();
}

size_t Can::available(){
    return rx_queue_.length();
}


Result<void, CanLibError> Can::try_write(const ClassicCanFrame & frame){
    // 注意这段代码不能改为直接往队列中存报文 
    // 因为如果没有报文被发送完成，中断一直不会被触发, 队列数据也就不会被外设消费

    const uint32_t temp_tstar = SPL_INST(p_inst_)->TSTATR;

    static constexpr uint32_t ANY_MAILBOX_IDLE_BITMASK = 
        (lld::can_tstatr_tme_mask(CanMailboxIndex::_0) 
        | lld::can_tstatr_tme_mask(CanMailboxIndex::_1) 
        | lld::can_tstatr_tme_mask(CanMailboxIndex::_2));


    const bool is_any_mailbox_idle = (temp_tstar & ANY_MAILBOX_IDLE_BITMASK) != 0;

    if(is_any_mailbox_idle){

        //根据数据手册 这里不可能为3
        const uint8_t idle_mbox_idx_bits = static_cast<uint8_t>(((temp_tstar) >> 24) & 0b11);
        if(idle_mbox_idx_bits > 2) __builtin_unreachable();

        const auto idle_mbox_idx = std::bit_cast<CanMailboxIndex>(idle_mbox_idx_bits);
        lld::can_transmit_nott(p_inst_, idle_mbox_idx, frame);
        return Ok();
    }else{
        //没有空闲邮箱，存入队列
        if(const auto write_quantity = tx_queue_.try_push(frame);
            write_quantity == 0){
            // 队列已满
            return Err(CanLibError::TxQueueOverflow);
        }
        return Ok();
    }
}

Option<ClassicCanFrame> Can::try_read(){
    Option<ClassicCanFrame> ret = None;

    if(const auto quantity = rx_queue_.consume_one([&](const ClassicCanFrame & frame){
        ret = Some(frame);
    }); quantity == 0) return None;
    return ret;
}


void CanIrqHandler::isr_tx(Can & self){
    volatile uint32_t & tstatr_reg = SPL_INST(self.p_inst_)->TSTATR;
    const bool callback_present = self.event_callback_ != nullptr;
    
    // 阶段1：处理中断事件（找RQCP置1的邮箱）

    static constexpr uint32_t ANY_TSTATR_RQCP_MASK =  lld::can_statr_rqcp_mask(hal::CanMailboxIndex::_0)
        | lld::can_statr_rqcp_mask(hal::CanMailboxIndex::_1)
        | lld::can_statr_rqcp_mask(hal::CanMailboxIndex::_2);


    uint32_t temp_tstatr = tstatr_reg;

    if(temp_tstatr & ANY_TSTATR_RQCP_MASK) [[likely]] {
        // 需要清除的标志位（RQCP/TXOK/ALST/TERR）
        uint32_t clear_mask = 0x0f;

        for(size_t i = 0; i < 3; i++){
            
            [[maybe_unused]] auto guard = make_scope_guard([&]{
                temp_tstatr >>= 8;
                clear_mask <<= 8;
            });

            bool is_rqcp_set = (temp_tstatr & 0x01) != 0;

            if(!is_rqcp_set){
                continue;
            }
    
            // 有RQCP，上报事件
            if(callback_present){
                uint8_t code = static_cast<uint8_t>(temp_tstatr & 0x0f);
                auto ev = CanEvent::from(hal::CanTransmitEvent{
                    .mbox_idx = std::bit_cast<hal::CanMailboxIndex>(uint8_t(i)),
                    .code = code
                });
                self.event_callback_(ev);
            }

            //写1清除RQCP/TXOK/ALST/TERR(写0无效)
            tstatr_reg = clear_mask;
        }
    }

    // 阶段2：处理队列补包（找TME置1的空闲邮箱）
    static constexpr uint32_t ANY_TME_MASK = lld::can_tstatr_tme_mask(CanMailboxIndex::_0)
        | lld::can_tstatr_tme_mask(CanMailboxIndex::_1)
        | lld::can_tstatr_tme_mask(CanMailboxIndex::_2);

    static constexpr size_t TME_BASE_SHIFT = __builtin_ctz(ANY_TME_MASK);

    //重新读取tstatr的值 因为调用回调函数可能会操作外设
    uint32_t temp_tme = tstatr_reg & ANY_TME_MASK;

    if(temp_tme == 0) return; // 无空闲邮箱，直接退出
    temp_tme >>= TME_BASE_SHIFT;

    size_t desired_dequeue_quantity = self.tx_queue_.length();

    for(size_t i = 0; i < 3; i++){
        [[maybe_unused]] auto guard = make_scope_guard([&]{
            temp_tme >>= 1;
        });

        // 不是空的
        if( (temp_tme & 0b01) == 0 ){
            continue;
        }

        // 空闲邮箱，补包
        if(desired_dequeue_quantity > 0){
            auto mbox_idx = std::bit_cast<hal::CanMailboxIndex>(uint8_t(i));
            auto quantity = self.tx_queue_.consume_one([&](const hal::ClassicCanFrame & frame){
                lld::can_transmit_nott(self.p_inst_, mbox_idx, frame);
            });
            desired_dequeue_quantity -= quantity;
        }else{
            break;
        }
    }
}


void CanIrqHandler::isr_rx(
    Can & self, 
    const CanFifoIndex fifo_idx
){
    volatile uint32_t & rfifo_reg = [&] -> volatile uint32_t &{
        switch(fifo_idx){
            case CanFifoIndex::_0: return can_get_rfifo_reg<CanFifoIndex::_0>(self.p_inst_);
            case CanFifoIndex::_1: return can_get_rfifo_reg<CanFifoIndex::_1>(self.p_inst_);
        }
        __builtin_unreachable();
    }();

    const uint32_t temp_rfifo_reg = rfifo_reg;

   // 1. 处理 FIFO 满
    if(temp_rfifo_reg & CAN_RFIFO_FFULL_MASK){
        [[maybe_unused]] auto isr_flag_clear_guard = make_scope_guard([&](){
            rfifo_reg = CAN_RFIFO_FFULL_MASK;
        });

        //rfifo满
        {
            const auto ev = CanEvent::from(hal::CanReceiveEvent{
                .fifo_idx = fifo_idx,
                .kind = CanReceiveEvent::Kind::FifoFull,
            });
            TRY_EMIT_EVENT(self, ev)
        }
    }
    
    // 2. 处理 FIFO 溢出
    if(temp_rfifo_reg & CAN_RFIFO_FOV_MASK){
        [[maybe_unused]] auto isr_flag_clear_guard = make_scope_guard([&](){
            rfifo_reg = CAN_RFIFO_FOV_MASK;
        });

        ///rfifo溢出
        {
            const auto ev = CanEvent::from(hal::CanReceiveEvent{
                .fifo_idx = fifo_idx,
                .kind = CanReceiveEvent::Kind::FifoOverrun,
            });
            TRY_EMIT_EVENT_OR_ABORT(self, ev, "can rxfifo overrun")
        }
    }

    // 3. FIFO中的报文入队
    {

        // 每次都访问寄存器以确保在中断发生过程中有新报文被收到 也能被处理
        auto get_latest_rfifo_pending_quantity = [&] -> size_t {
            const uint32_t readed_rfifo_reg = rfifo_reg;
            return static_cast<size_t>((readed_rfifo_reg & CAN_RFIFO_FMP_MASK) >> 0);
        };

        auto notify_frame_readed = [&](){
            rfifo_reg = CAN_RFIFO_FOM_MASK;
        };

        size_t rfifo_pending_quantity = get_latest_rfifo_pending_quantity();
        //注意这里是判断fmp掩码 但是清零的是fom
        while(rfifo_pending_quantity){

            //入队的报文数量
            const size_t enqueued_quantity = self.rx_queue_.try_push(
                //取走一个报文
                lld::can_receive(self.p_inst_, fifo_idx)
            );

            //总是清除中断标志 不管接收到的消息是否能够入队
            //如不清除 程序将无法逃离中断
            notify_frame_readed();
            
            if(enqueued_quantity){
                const auto ev = CanEvent::from(hal::CanReceiveEvent{
                    .fifo_idx = fifo_idx,
                    .kind = CanReceiveEvent::Kind::FrameEnqueued,
                });
                TRY_EMIT_EVENT(self, ev)
                rfifo_pending_quantity -= enqueued_quantity;
            }else{
                self.queue_ovf_count.fetch_add(1u, std::memory_order_relaxed);
                const auto ev = CanEvent::from(hal::CanReceiveEvent{
                    .fifo_idx = fifo_idx,
                    .kind = CanReceiveEvent::Kind::FrameEnqueueFailed,
                });
                TRY_EMIT_EVENT(self, ev)
                break;
            }

            //临别之前确保中断过程中收到的报文也被消费 避免频繁进入中断
            //理论上如果频繁被置位会导致死循环 但实际由于can报文最短接收间隔也有数十微秒 不会造成这样的问题
            if(rfifo_pending_quantity == 0)
                rfifo_pending_quantity = get_latest_rfifo_pending_quantity();
        }
    }
}

__attribute__((hot, flatten))
void CanIrqHandler::isr_sce(Can & self) {
    //TODO 不再逐位判断 而是将连续的位一起打包

    void* p_inst = self.p_inst_;

    const uint32_t temp_inten = SPL_INST(p_inst)->INTENR;
    const uint32_t temp_statr = SPL_INST(p_inst)->STATR;
    const uint32_t temp_errsr = SPL_INST(p_inst)->ERRSR;

    auto flag_bits = hal::CanStatusFlag::zero();

    if(temp_inten & (CAN_IT_WKU | CAN_IT_SLK | CAN_IT_ERR)){
        // Wake-up interrupt (WKU)
        if ((temp_inten & CAN_IT_WKU) && (temp_statr & CAN_STATR_WKUI)) {
            flag_bits.wakeup = 1;
            SPL_INST(p_inst)->STATR = CAN_STATR_WKUI;  // W1C
        }
    
        // Sleep acknowledge interrupt (SLK)
        if ((temp_inten & CAN_IT_SLK) && (temp_statr & CAN_STATR_SLAKI)) {
            flag_bits.sleep_acknowledge = 1;
            SPL_INST(p_inst)->STATR = CAN_STATR_SLAKI;  // W1C
        }
    
        // General error interrupt (ERR)
        if ((temp_inten & CAN_IT_ERR) && (temp_statr & CAN_STATR_ERRI)) {
            flag_bits.error = 1;
            SPL_INST(p_inst)->ERRSR = RESET;
            SPL_INST(p_inst)->STATR = CAN_STATR_ERRI;  // W1C
        }
    }

    if(temp_inten & (CAN_IT_LEC | CAN_IT_EWG | CAN_IT_EPV | CAN_IT_BOF)){
        // Error warning (EWG)
        if ((temp_inten & CAN_IT_EWG) && (temp_errsr & CAN_ERRSR_EWGF)) {
            flag_bits.error_warning = 1;
            SPL_INST(p_inst)->STATR = CAN_STATR_ERRI;  // W1C (shared ERR flag)
        }

        // Error passive (EPV)
        if ((temp_inten & CAN_IT_EPV) && (temp_errsr & CAN_ERRSR_EPVF)) {
            flag_bits.error_passive = 1;
            SPL_INST(p_inst)->STATR = CAN_STATR_ERRI;  // W1C
        }

        // Bus-off (BOF)
        if ((temp_inten & CAN_IT_BOF) && (temp_errsr & CAN_ERRSR_BOFF)) {
            flag_bits.bus_off = 1;
            SPL_INST(p_inst)->STATR = CAN_STATR_ERRI;  // W1C
        }

        // Last error code (LEC)
        if ((temp_inten & CAN_IT_LEC) && (temp_errsr & CAN_ERRSR_LEC)) {
            flag_bits.last_error_code = 1;
            SPL_INST(p_inst)->ERRSR = RESET;
            SPL_INST(p_inst)->STATR = CAN_STATR_ERRI;
        }
    }


    TRY_EMIT_EVENT(self, CanEvent::from(flag_bits));
}