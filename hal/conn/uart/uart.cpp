#include "uart.hpp"
#include "uart_layout.hpp"
#include "ral/uart.hpp"

#include "hal/dma/dma.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/sdk.hpp"
#include "core/intrinsics/volatile.hpp"
#include "core/utils/scope_guard.hpp"

using namespace ymd;
using namespace ymd::hal;


static constexpr const NvicPriorityCode UART_RX_DMA_INTERRUPT_NVIC_PRIORITY 
    = NvicPriorityCode::from_pre_sub_d2(1,0);

static constexpr const NvicPriorityCode UART_TX_DMA_INTERRUPT_NVIC_PRIORITY 
    = NvicPriorityCode::from_pre_sub_d2(1,2);

static constexpr const NvicPriorityCode UART_INTERRUPT_NVIC_PRIORITY 
    = NvicPriorityCode::from_pre_sub_d2(1,1);

static constexpr const DmaPriority RX_DMA_DMA_PRIORITY = DmaPriority::Medium;
static constexpr const DmaPriority TX_DMA_DMA_PRIORITY = DmaPriority::Medium;


static_assert(std::has_single_bit(UART_RX_DMA_BUF_SIZE)); //缓冲区大小必须是2的幂
static_assert(std::has_single_bit(UART_TX_DMA_BUF_SIZE)); //缓冲区大小必须是2的幂

static constexpr size_t HALF_UART_RX_DMA_BUF_SIZE = UART_RX_DMA_BUF_SIZE / 2;


using Event = Uart::Event;

#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SPL_INST(x) (reinterpret_cast<COPY_CONST(x, USART_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::USART_Def)>(x))


#define TRY_EMIT_EVENT(self, x)   \
if(self.event_callback_ != nullptr) {\
    self.event_callback_(x);\
}\


#ifdef RELEASE


#define TRY_EMIT_EVENT_OR_ABORT(self, x, str)\
if(self.event_callback_ != nullptr){\
    self.event_callback_(x);\
} else{\
}\


#define ABORT(str) {};
#define UNREACHABLE() __builtin_unreachable();

#else

#define TRY_EMIT_EVENT_OR_ABORT(self, x, str)\
if(self.event_callback_ != nullptr){\
    self.event_callback_(x);\
} else{\
    UNREACHABLE();\
}\


#define ABORT(str)\
sys::abort(AbortInfo::from_reason(str));\

#define UNREACHABLE() __builtin_trap();


#endif

namespace {

#define UNUSED(x) (void)(x) 

#if 1
template<typename T>
static T clone_volatile(volatile T * p_reg){
    return *const_cast<T *>(p_reg);
}

template<typename T>
static void notify_volatile_readed(volatile T * p_reg){
    const T dummy = *const_cast<T *>(p_reg);
    UNUSED(dummy);
    return;
}

template<typename T>
static void store_volatile(volatile T * p_reg, const auto x){
    *const_cast<T *>(p_reg) = std::bit_cast<T>(x);
}


#endif


template<UartRemap REMAP>
[[maybe_unused]] static Gpio _uart_to_tx_pin(const Nth nth){
    switch(nth.count()){
        #ifdef USART1_PRESENT
        case 1:
            return pintag_to_pin<uart::tx_pin_t<1, REMAP>>();
        #endif
        #ifdef USART2_PRESENT
        case 2:
            return pintag_to_pin<uart::tx_pin_t<2, REMAP>>();
        #endif
        #ifdef USART3_PRESENT
        case 3:
            return pintag_to_pin<uart::tx_pin_t<3, REMAP>>();
        #endif
        #ifdef UART4_PRESENT
        case 4:
            return pintag_to_pin<uart::tx_pin_t<4, REMAP>>();
        #endif
        #ifdef UART5_PRESENT
        case 5:
            return pintag_to_pin<uart::tx_pin_t<5, REMAP>>();
        #endif
        #ifdef UART6_PRESENT
        case 6:
            return pintag_to_pin<uart::tx_pin_t<6, REMAP>>();
        #endif
        #ifdef UART7_PRESENT
        case 7:
            return pintag_to_pin<uart::tx_pin_t<7, REMAP>>();
        #endif
        #ifdef UART8_PRESENT
        case 8:
            return pintag_to_pin<uart::tx_pin_t<8, REMAP>>();
        #endif
    }
    UNREACHABLE();
}

template<UartRemap REMAP>
[[maybe_unused]] static Gpio _uart_to_rx_pin(const Nth nth){
    switch(nth.count()){
        #ifdef USART1_PRESENT
        case 1:
            return pintag_to_pin<uart::rx_pin_t<1, REMAP>>();
        #endif
        #ifdef USART2_PRESENT
        case 2:
            return pintag_to_pin<uart::rx_pin_t<2, REMAP>>();
        #endif
        #ifdef USART3_PRESENT
        case 3:
            return pintag_to_pin<uart::rx_pin_t<3, REMAP>>();
        #endif
        #ifdef UART4_PRESENT
        case 4:
            return pintag_to_pin<uart::rx_pin_t<4, REMAP>>();
        #endif
        #ifdef UART5_PRESENT
        case 5:
            return pintag_to_pin<uart::rx_pin_t<5, REMAP>>();
        #endif
        #ifdef UART6_PRESENT
        case 6:
            return pintag_to_pin<uart::rx_pin_t<6, REMAP>>();
        #endif
        #ifdef UART7_PRESENT
        case 7:
            return pintag_to_pin<uart::rx_pin_t<7, REMAP>>();
        #endif
        #ifdef UART8_PRESENT
        case 8:
            return pintag_to_pin<uart::rx_pin_t<8, REMAP>>();
        #endif
    }
    UNREACHABLE();
}


#define DEF_UART_BIND_PIN_LAYOUTER(name)\
[[maybe_unused]] static Gpio uart_to_##name##_pin(const Nth nth, const UartRemap remap){\
    switch(remap){\
        case UartRemap::_0: return _uart_to_##name##_pin<UartRemap::_0>(nth);\
        case UartRemap::_1: return _uart_to_##name##_pin<UartRemap::_1>(nth);\
        case UartRemap::_2: return _uart_to_##name##_pin<UartRemap::_2>(nth);\
        case UartRemap::_3: return _uart_to_##name##_pin<UartRemap::_3>(nth);\
    }\
    UNREACHABLE();\
}\

DEF_UART_BIND_PIN_LAYOUTER(tx)
DEF_UART_BIND_PIN_LAYOUTER(rx)

static DmaChannel & _uart_to_rx_dma(const Nth nth){
    switch(nth.count()){
        #ifdef USART1_PRESENT
        case 1:
            return USART1_RX_DMA_CH;
        #endif
        #ifdef USART2_PRESENT
        case 2:
            return USART2_RX_DMA_CH;
        #endif
        #ifdef USART3_PRESENT
        case 3:
            return USART3_RX_DMA_CH;
        #endif
        #ifdef UART4_PRESENT
        case 4:
            return UART4_RX_DMA_CH;
        #endif
        #ifdef UART5_PRESENT
        case 5:
            return UART5_RX_DMA_CH;
        #endif
        #ifdef UART6_PRESENT
        case 6:
            return UART6_RX_DMA_CH;
        #endif
        #ifdef UART7_PRESENT
        case 7:
            return UART7_RX_DMA_CH;
        #endif
        #ifdef UART8_PRESENT
        case 8:
            return UART8_RX_DMA_CH;
        #endif
    }
    UNREACHABLE();
}


static DmaChannel & _uart_to_tx_dma(const Nth nth){
    switch(nth.count()){
        #ifdef USART1_PRESENT
        case 1:
            return USART1_TX_DMA_CH;
        #endif
        #ifdef USART2_PRESENT
        case 2:
            return USART2_TX_DMA_CH;
        #endif
        #ifdef USART3_PRESENT
        case 3:
            return USART3_TX_DMA_CH;
        #endif
        #ifdef UART4_PRESENT
        case 4:
            return UART4_TX_DMA_CH;
        #endif
        #ifdef UART5_PRESENT
        case 5:
            return UART5_TX_DMA_CH;
        #endif
        #ifdef UART6_PRESENT
        case 6:
            return UART6_TX_DMA_CH;
        #endif
        #ifdef UART7_PRESENT
        case 7:
            return UART7_TX_DMA_CH;
        #endif
        #ifdef UART8_PRESENT
        case 8:
            return UART8_TX_DMA_CH;
        #endif
    }
    UNREACHABLE();
}

static constexpr uint32_t STATR_CLEARABLE_MASK = (0b11'0110'0000);
}


Uart::Uart(
    void * inst
):
    p_inst_(inst),
    inst_nth_(lld::uart_to_nth(reinterpret_cast<uintptr_t>(p_inst_))),
    tx_dma_(_uart_to_tx_dma(inst_nth_)),
    rx_dma_(_uart_to_rx_dma(inst_nth_)){;}




static constexpr uint32_t calc_buadrate_hz(hal::UartBaudrate baudrate){ 
    const auto baudrate_hz = [&] -> uint32_t{
        if(baudrate.is<hal::NearestFreq>()){
            return baudrate.unwrap_as<hal::NearestFreq>().count;
        }else{
            UNREACHABLE();
        }
    }();

    return baudrate_hz;
}


void Uart::init(const Config & cfg){
    [[maybe_unused]] auto & self = *this;

    enable_rcc(EN);

    USART_Cmd(SPL_INST(p_inst_), DISABLE);

    set_remap(cfg.remap);


    //setup tx pin
    if(cfg.tx_strategy != CommStrategy::Disabled){
        auto tx_pin = uart_to_tx_pin(inst_nth_, cfg.remap);
        tx_pin.afpp();
    }

    //setup rx pin
    if(cfg.rx_strategy != CommStrategy::Disabled){
        auto rx_pin = uart_to_rx_pin(inst_nth_, cfg.remap);

        //串口的物理层是低有效
        rx_pin.inpu();
        // rx_pin.inflt();
    }

    const uint32_t baudrate_hz = calc_buadrate_hz(cfg.baudrate);

    
    {
        const USART_InitTypeDef USART_InitStructure{
            .USART_BaudRate = baudrate_hz,
            .USART_WordLength = USART_WordLength_8b,
            .USART_StopBits = USART_StopBits_1,
            .USART_Parity = USART_Parity_No,
            .USART_Mode = ({
                uint16_t mode_mask = 0;
                if(cfg.tx_strategy != CommStrategy::Disabled) mode_mask |= USART_Mode_Tx;
                if(cfg.rx_strategy != CommStrategy::Disabled) mode_mask |= USART_Mode_Rx;
                mode_mask;
            }),
            .USART_HardwareFlowControl = USART_HardwareFlowControl_None
        };

        USART_Init(SPL_INST(p_inst_), &USART_InitStructure);
    }



    #if 1
    //清除中断标志位
    if(1){
        //清除statr标志位
        store_volatile(reinterpret_cast<volatile uint32_t*>(&SPL_INST(self.p_inst_)->STATR)
            ,  ~STATR_CLEARABLE_MASK
        );

        //清除部分要求先读statr再读datar可以清除的标志位
        RAL_INST((self).p_inst_)->STATR;
        RAL_INST((self).p_inst_)->DATAR;
    }
    #endif


    lld::usart_enable_error_interrupt(p_inst_, EN);
    register_nvic(UART_INTERRUPT_NVIC_PRIORITY, EN);

    set_tx_strategy(cfg.tx_strategy);
    set_rx_strategy(cfg.rx_strategy);

    USART_Cmd(SPL_INST(p_inst_), ENABLE);
}

void Uart::enable_tx(const Enable en){
    RAL_INST(p_inst_)->CTLR1.TE = (en == EN);
}


void Uart::enable_rx(const Enable en){
    RAL_INST(p_inst_)->CTLR1.RE = (en == EN);
}

size_t Uart::try_write_bytes(const std::span<const uint8_t> bytes){
    switch(tx_strategy_){
        case CommStrategy::Blocking:{
            auto * p_byte = bytes.data();
            auto * p_end = bytes.data() + bytes.size();

            while(p_byte < p_end){
                while((RAL_INST(p_inst_)->STATR.TXE) == RESET);
                RAL_INST(p_inst_)->DATAR.DR = static_cast<uint32_t>(*p_byte);
                p_byte ++;
            }

            return bytes.size();
        }
        case CommStrategy::Interrupt:{
            const auto written_quantity = tx_queue_.try_push(bytes);
            enable_tx_interrupt(EN);
            return written_quantity;
        }
        case CommStrategy::Dma:{
            const auto written_quantity = tx_queue_.try_push(bytes);
            poll_tx_dma();
            return written_quantity;
        }
        case CommStrategy::Disabled:
            //运行到这里说明你可能配置错串口了 让未启用输出的串口输出数据
            UNREACHABLE();
            return 0;
    }
    //无法到达这个控制流
    UNREACHABLE();
}

size_t Uart::try_write_byte(const uint8_t byte){
    return try_write_bytes(std::span(&byte, 1));
}

void Uart::deinit(){
    enable_rcc(DISEN);
}

void Uart::enable_rcc(const Enable en){
    lld::uart_enable_rcc(inst_nth_, en);
}


void Uart::set_remap(const UartRemap remap){
    lld::uart_set_remap(inst_nth_, remap);
}

void Uart::enable_single_line_mode(const Enable en){
    USART_HalfDuplexCmd(SPL_INST(p_inst_), (en == EN));
}


void Uart::register_nvic(hal::NvicPriorityCode priority, const Enable en){
    const auto irqn = lld::uart_calc_nvic_irqn(inst_nth_);
    lld::nvic_set_irqn_priority(irqn, priority);
    lld::nvic_enable_irqn(irqn, en == EN);
}

void Uart::set_tx_strategy(const CommStrategy tx_strategy){
    if(tx_strategy_ == tx_strategy) return;
    auto guard = make_scope_guard([&]{
        tx_strategy_ = tx_strategy;
    });

    if(tx_strategy == CommStrategy::Disabled){
        enable_tx(DISEN);
        enable_tx_dma(DISEN);
        enable_tx_interrupt(DISEN);
        return;
    }

    enable_tx(EN);

    switch(tx_strategy){
        case CommStrategy::Disabled:
            __builtin_unreachable();
            break;
        case CommStrategy::Blocking:
            enable_tx_dma(DISEN);
            enable_tx_interrupt(DISEN);
            break;
        case CommStrategy::Interrupt:
            enable_tx_dma(DISEN);
            enable_tx_interrupt(EN);
            break;
        case CommStrategy::Dma:
            enable_tx_dma(EN);
            setup_tx_dma(TX_DMA_DMA_PRIORITY);
            enable_tx_interrupt(DISEN);
            break;
    }
}

void Uart::poll_tx_dma(){
    auto & self = *this;
    if(tx_dma_.pending_count() == 0){
        const size_t req_dequeue_quantity = tx_queue_.length();
        if(req_dequeue_quantity){
            const size_t act_dequeue_quantity = tx_queue_.try_pop(
                std::span(tx_dma_buf_.begin(), req_dequeue_quantity)
            );
            tx_dma_.start_transfer_mem2pph<DmaWordSize::OneByte, DmaWordSize::OneByte>(
                (&SPL_INST(self.p_inst_)->DATAR), 
                tx_dma_buf_.begin(), 
                act_dequeue_quantity
            );
        }else{
            {
                const auto uev = Event::TxIdle;
                TRY_EMIT_EVENT(self, uev);
            }
        }
    }

    #if 0
    [this](const DmaEvent ev){
        auto & self = *this;
        switch(ev){
        case DmaEvent::TransferComplete:{
            //将数据从当前索引填充至末尾
            const size_t quantity = self.tx_queue_.try_pop(std::span(
                &tx_dma_buf_[tx_dma_buf_index_],
                UART_TX_DMA_BUF_SIZE - tx_dma_buf_index_
            ));

            (void)quantity;
            tx_dma_buf_index_ = 0;
            break;
        }
        case DmaEvent::TransferOnhalf:{
            //将数据从当前索引填充至半满
            const size_t quantity = self.tx_queue_.try_pop(std::span(
                &tx_dma_buf_[tx_dma_buf_index_],
                (UART_TX_DMA_BUF_SIZE / 2) - tx_dma_buf_index_
            ));

            (void)quantity;
            tx_dma_buf_index_ = UART_TX_DMA_BUF_SIZE / 2;
            break;
        }

        default:
            break;
        }
    }
    #endif
}

void Uart::setup_tx_dma(const DmaPriority priority){
    // TODO 优化调度为环形模式

    // 这是一个简单的txdma调度 但是至少不会丢包粘包 
    // 每次在缓冲区起始存入数据 然后触发dma开始搬运
    
    tx_dma_.init({
        .mode = DmaMode::BurstMemoryToPeriph,
        .priority = priority
    });


    tx_dma_.set_event_callback([this](const DmaEvent ev){
        UNUSED(ev);
        poll_tx_dma();
    });

    tx_dma_.register_nvic(UART_TX_DMA_INTERRUPT_NVIC_PRIORITY, EN);

    tx_dma_.enable_interrupt<DmaIT::Done>(EN);
    // tx_dma_.enable_interrupt<DmaIT::Half>(EN);
}



void Uart::enable_tx_dma(const Enable en){
    RAL_INST(p_inst_)->CTLR3.DMAT = (en == EN);
}

void Uart::enable_rx_dma(const Enable en){
    RAL_INST(p_inst_)->CTLR3.DMAR = (en == EN);
}


void Uart::set_rx_strategy(const CommStrategy rx_strategy){
    if(rx_strategy_ == rx_strategy) return;
    auto guard = make_scope_guard([&]{
        rx_strategy_ = rx_strategy;
    });

    if(rx_strategy == CommStrategy::Disabled){
        enable_rx(DISEN);
        enable_rx_dma(DISEN);
        enable_idle_interrupt(DISEN);
        enable_rxne_interrupt(DISEN);
        return;
    }

    enable_rx(EN);

    switch(rx_strategy){
        case CommStrategy::Disabled:
            //handled above
            __builtin_unreachable();
            break;
        case CommStrategy::Blocking:
            //blocking receive ? You idiot!!
            ABORT("idiot");
            break;
        case CommStrategy::Interrupt:
            enable_rx_dma(DISEN);
            enable_idle_interrupt(DISEN);
            enable_rxne_interrupt(EN);
            break;
        case CommStrategy::Dma:
            enable_rxne_interrupt(DISEN);
            enable_idle_interrupt(EN);
            enable_rx_dma(EN);
            setup_rx_dma(RX_DMA_DMA_PRIORITY);
            break;
    }
    rx_strategy_ = rx_strategy;

}


void Uart::setup_rx_dma(const DmaPriority priority){

    rx_dma_.init({
        .mode = DmaMode::PeriphToBurstMemoryCircular,
        .priority = priority
    });


    rx_dma_.set_event_callback(
        [this](const DmaEvent ev) -> void{
            auto & self = *this;
            const auto rx_dma_buf_index = self.rx_dma_buf_index_;

            switch(ev){
            case DmaEvent::TransferComplete:{
                //先消费队列中滞留的数据 再填充
                {
                    const auto uev = Event::RxBulk;
                    TRY_EMIT_EVENT(self, uev);
                }
                
                // DMA传送后半部分完成 将后半部分的dma缓冲区填入fifo中

                // [          |######   ]
                //                  ^ rx_dma_buf_index

                // [          |#########]
                // ^ rx_dma_buf_index

                const size_t required_quantity = (HALF_UART_RX_DMA_BUF_SIZE * 2) - rx_dma_buf_index;

                const size_t actual_quantity = self.rx_queue_.try_push(std::span(
                    &rx_dma_buf_[rx_dma_buf_index],
                    required_quantity
                ));

                if(actual_quantity < required_quantity){
                    // 接收队列无法继续存全部数据 可能是接收的数据没有被及时读取导致的
                    const auto uev = Event::RxDmaTcOverflow;
                    TRY_EMIT_EVENT_OR_ABORT(self, uev, "uart rx(dma tc) queue overflow");
                }

                self.rx_dma_buf_index_ = 0;
            }
                break;
            case DmaEvent::TransferOnhalf:{
                //先消费队列中滞留的数据 再填充
                {
                    const auto uev = Event::RxBulk;
                    TRY_EMIT_EVENT(self, uev);
                }

                // DMA传送前半部分完成 将前半部分的dma缓冲区填入fifo中

                // [######   |         ]
                //       ^ rx_dma_buf_index

                // [#########|         ]
                //           ^ rx_dma_buf_index

                const size_t required_quantity = HALF_UART_RX_DMA_BUF_SIZE - rx_dma_buf_index;
                const size_t actual_quantity = self.rx_queue_.try_push(std::span(
                    &rx_dma_buf_[rx_dma_buf_index],
                    required_quantity
                ));

                if(actual_quantity < required_quantity){
                    // 接收队列无法继续存全部数据 可能是接收的数据没有被及时读取导致的
                    const auto uev = Event::RxDmaHcOverflow;
                    TRY_EMIT_EVENT_OR_ABORT(self, uev, "uart rx(dma hc) queue overflow");
                }

                self.rx_dma_buf_index_ = HALF_UART_RX_DMA_BUF_SIZE;
            }
                break;
            default:
                break;
            }
        }
    );

    rx_dma_.register_nvic(UART_RX_DMA_INTERRUPT_NVIC_PRIORITY, EN);
    rx_dma_.enable_interrupt<DmaIT::Done>(EN);
    rx_dma_.enable_interrupt<DmaIT::Half>(EN);

    rx_dma_.start_transfer_pph2mem<
        DmaWordSize::OneByte, 
        DmaWordSize::OneByte
    >(
        rx_dma_buf_.data(),
        &RAL_INST(p_inst_)->DATAR.DR,
        UART_RX_DMA_BUF_SIZE
    );
}

void UartIrqHandler::isr_rxne(Uart & self){
    switch(self.rx_strategy_){
        case CommStrategy::Dma:{
            break;
        }
        case CommStrategy::Interrupt:{
            const auto byte = static_cast<uint8_t>(RAL_INST(self.p_inst_)->DATAR.DR);

            if(const auto quantity = self.rx_queue_.try_push(byte);
                quantity == 0
            ){
                const auto uev = Event::RxQueueOverflow;
                TRY_EMIT_EVENT_OR_ABORT(self, uev, "uart rx(int) queue overflow");
                // 接收的数据没有被及时读取 接收队列无法继续存数据
            }
        }
            break;
        default:
            break;
    }

}

void UartIrqHandler::isr_tc(Uart & self){
    //TODO
    UNUSED(self);
}

void UartIrqHandler::isr_txe(Uart & self){
    switch(self.tx_strategy_){
        case CommStrategy::Dma:
            break;
        case CommStrategy::Interrupt:{
            uint8_t byte;
            if(const auto quantity = self.tx_queue_.try_pop(byte);
                quantity != 0){
                SPL_INST(self.p_inst_)->DATAR = byte;
            }
        }
            break;
        default:
            break;
    }
}

void UartIrqHandler::isr_idle(Uart & self){

    auto emit_idle_event = [&self](){
        const auto uev = Event::RxIdle;
        TRY_EMIT_EVENT(self, uev);

    };

    switch(self.rx_strategy_){
        case CommStrategy::Dma:{
            const size_t supposed_dma_buf_index = UART_RX_DMA_BUF_SIZE - self.rx_dma_.pending_count();

            if(supposed_dma_buf_index >= UART_RX_DMA_BUF_SIZE) [[unlikely]]
                UNREACHABLE();

            const uint32_t half_unaligned_mask = (HALF_UART_RX_DMA_BUF_SIZE - 1);

            // 如果这个索引已经被对齐到全部传输完成或者一半传输完成 
            // dma半满和全满中断将会接管缓冲填充操作 
            if(supposed_dma_buf_index & half_unaligned_mask) [[likely]] {
                const auto rx_dma_buf_index = self.rx_dma_buf_index_;
                const auto required_quantity = size_t(supposed_dma_buf_index - rx_dma_buf_index);
                const auto actual_quantity = self.rx_queue_.try_push(std::span(
                    self.rx_dma_buf_.data() + rx_dma_buf_index, required_quantity
                ));

                if(actual_quantity != required_quantity){
                    // 接收的数据没有被及时读取 接收队列无法继续存数据
                    const auto uev = Event::RxDmaHcOverflow;
                    TRY_EMIT_EVENT_OR_ABORT(self, uev, "uart rx(dma hc) queue overflow");
                }
            }

            self.rx_dma_buf_index_ = supposed_dma_buf_index;
            emit_idle_event();
        };
            break;

        case CommStrategy::Interrupt:{
            emit_idle_event();
            break;
        }

        case CommStrategy::Disabled:{
            ABORT("uart rx disabled, but triggered idle")
        }

        default:
            break;
    }
}


void Uart::enable_rxne_interrupt(const Enable en){
    lld::uart_enable_idle_interrupt(p_inst_, en);
}

void Uart::enable_tx_interrupt(const Enable en){
    lld::uart_enable_tx_interrupt(p_inst_, en);
}


void Uart::enable_idle_interrupt(const Enable en){
    lld::uart_enable_idle_interrupt(p_inst_, en);
}

struct alignas(4) [[nodiscard]] BareUartEvent final{
    enum class Bits:uint32_t{
        ParityError = 1u << 0,
        FrameError = 1u << 1,
        NoiseError = 1u << 2,
        OverrunError = 1u << 3,
        Idle = 1u << 4,
        RxNotEmpty = 1u << 5,
        TxClear = 1u << 6,
        TxEmpty = 1u << 7,
        LinBreakDetected = 1u << 8,
        CtsStateChange = 1u << 9
    };

    uint32_t parity_error:1;
    uint32_t frame_error:1;
    uint32_t noise_error:1;
    uint32_t overrun_error:1;

    uint32_t idle:1;
    uint32_t rx_not_empty:1;
    uint32_t tx_clear:1;
    uint32_t tx_empty:1;
    uint32_t lin_break_detected:1;
    uint32_t cts_state_change:1;
};


void UartIrqHandler::on_interrupt(Uart & self){
    auto * ral_inst = RAL_INST(self.p_inst_);
    const auto temp_statr = clone_volatile(&ral_inst->STATR);
    const auto temp_ctlr1 = clone_volatile(&ral_inst->CTLR1);

    // statr
    // |  3  |  2   |  1 |  0   |
    // | ore |  ne  | fe |  pe  |

    // ore:过载错误
    // ne:噪声错误
    // fe:帧错误
    // pe:奇偶校验错误

    if(std::bit_cast<uint32_t>(temp_statr) & 0x0f){
        // 过载错误标志。当接收移位寄存器存在数据需
        // 要转到数据寄存器时，但是数据寄存器的接收
        // 域还有数据未读出时，此位将会被置位。如果
        // RXNEIE 被置位了，还会产生对应中断
        if(temp_statr.ORE){
            {
                //TODO
            }
            //这个事件无法自然消退
        }

        // 帧错误标志。当检测到同步错误，过多的噪声
        // 或者断开符，该位将会被硬件置位。读此位再
        // 读数据寄存器的操作会复位此位。
        if(temp_statr.FE){
            //帧错误
            {
                //TODO
            }
            //这个事件无法自然消退
        }

        // 校验错误标志。在接收模式下，如果产生奇偶
        // 检验错误，硬件置位此位。读此位再读数据寄
        // 存器的操作会复位此位。在清除此位前，软件
        // 必须等 RXNE 标志位被置位。如果 PEIE 之前已
        // 经被置位，那么此位被置位会产生对应的中
        // 断。
        if(temp_statr.PE){
            //奇偶校验位错误
            {
                //TODO
            }

            notify_volatile_readed(&ral_inst->DATAR);
        }

        // 噪声错误标志。当检测到噪声错误标志时，由
        // 硬件置位。读状态寄存器后，再读数据寄存器
        // 的操作会复位此位。
        if(temp_statr.NE){
            // 噪声错误标志
            {
                //TODO
            }

            notify_volatile_readed(&ral_inst->DATAR);
        }
    }


    // ctlr1和statr在这部分共用相同字段
    // |  7  |  6   |  5   |  4  |
    // | txe |  tc  | rxne |  idle  |
    const uint32_t transmission_flags_mask = 
        std::bit_cast<uint32_t>(temp_ctlr1) & 
        std::bit_cast<uint32_t>(temp_statr) & 0xf0;

    if(transmission_flags_mask){
        // rxne
        // 读数据寄存器非空标志，当移位寄存器中的数
        // 据被转移到数据寄存器中，该位会被硬件置
        // 位。如果 RXNEIE 已经被置位，则还会产生对应
        // 的中断。对数据寄存器的读操作可以将该位清
        // 除。也可以直接写 0 来清除该位。
        if(transmission_flags_mask & (1u << 5)){
            UartIrqHandler::isr_rxne(self);

            // 也可以直接写 0 来清除该位
            // store_volatile(&ral_inst->STATR, (~(1u << 5)));
        }

        // txe
        // 发送数据寄存器空标志。当 TDR 寄存器中的的
        // 数据被硬件转移到移位寄存器的时候，该位被
        // 硬件置位。如果 TXEIE 已经被置位时，就会产
        // 生中断，对数据寄存器进行写操作，此位将会
        // 被复位。
        if(transmission_flags_mask & (1u << 7)){
            UartIrqHandler::isr_txe(self);
            // 对数据寄存器进行写操作，此位将会
            // 被复位。
        }

        // tc
        // 发送完成标志。当含有数据的一帧发送完成
        // 后，并且 TXE 被置位，则硬件将会此位置位，
        // 如果 TCIE 被置位，还会产生对应中断，软件读
        // 了此位再写数据寄存器则会清除此位。也可以
        // 直接写 0 来清除此位。
        if(transmission_flags_mask & (1u << 6)){
            UartIrqHandler::isr_tc(self);

            // 也可以直接写 0 来清除此位
            store_volatile(&ral_inst->STATR, (~(1u << 6)));
            // USART_ClearITPendingBit(SPL_INST(self.p_inst_), USART_IT_TC);
        }

        // idle
        // 总线空闲标志。当总线空闲时，该位将会被硬
        // 件置位。如果 IDLEIE 已经被置位，则会产生对
        // 应的中断。读状态寄存器再读数据寄存器的操
        // 作会清除此位。
        if(transmission_flags_mask & (1u << 4)){
            UartIrqHandler::isr_idle(self);

            //已经读了状态寄存器了，
            // 现在再读数据寄存器
            // notify_volatile_readed(&ral_inst->DATAR);
            // ral_inst->STATR;
            ral_inst->DATAR;
        }
    }


    #if 0
    if(temp_statr.LBD) [[unlikely]]{
        const auto temp_ctlr2 = clone_volatile(&ral_inst->CTLR2);
        // LIN Break 检测标志。当检测到 LIN Break 时，
        // 该位被硬件置位。由软件清零。如果 LBDIE 已
        // 经被置位，则将会产生中断。
        if(temp_ctlr2.LBDIE){

            //TODO
        }
    }

    if(temp_statr.CTS) [[unlikely]]{
        const auto temp_ctlr3 = clone_volatile(&ral_inst->CTLR3);
        
        // CTS 状态改变标志。如果设置了 CTSE 位，当
        // nCTS 输出状态改变时，该位将由硬件置高。由
        // 软件清零。如果 CTSIE 位已经被置位，则会产
        // 生中断。
        if(temp_ctlr3.CTSIE){

            //TODO
        }
    }
    #endif

};


