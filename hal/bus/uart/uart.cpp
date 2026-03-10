#include "uart.hpp"
#include "uart_layout.hpp"
#include "ral/uart.hpp"

#include "hal/dma/dma.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/sdk.hpp"


using namespace ymd;
using namespace ymd::hal;


static constexpr const NvicPriority UART_RX_DMA_INTERRUPT_NVIC_PRIORITY = {1,4};
static constexpr const NvicPriority UART_TX_DMA_INTERRUPT_NVIC_PRIORITY = {1,5};
static constexpr const NvicPriority UART_INTERRUPT_NVIC_PRIORITY = {1,3};

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

#define SDK_INST(x) (reinterpret_cast<COPY_CONST(x, USART_TypeDef)>(x))
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


#define RAISE_ABORT_ERR(str) {};

#else

#define TRY_EMIT_EVENT_OR_ABORT(self, x, str)\
if(self.event_callback_ != nullptr){\
    self.event_callback_(x);\
} else{\
    __builtin_trap();\
}\


#define RAISE_ABORT_ERR(str)\
sys::abort(AbortInfo::from_reason(str));\

#endif
namespace {


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
    __builtin_trap();
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
    __builtin_trap();
}


#define DEF_UART_BIND_PIN_LAYOUTER(name)\
[[maybe_unused]] static Gpio uart_to_##name##_pin(const Nth nth, const UartRemap remap){\
    switch(remap){\
        case UartRemap::_0: return _uart_to_##name##_pin<UartRemap::_0>(nth);\
        case UartRemap::_1: return _uart_to_##name##_pin<UartRemap::_1>(nth);\
        case UartRemap::_2: return _uart_to_##name##_pin<UartRemap::_2>(nth);\
        case UartRemap::_3: return _uart_to_##name##_pin<UartRemap::_3>(nth);\
    }\
    __builtin_trap();\
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
    __builtin_trap();
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
    __builtin_trap();
}


}


Uart::Uart(
    void * inst
):
    p_inst_(inst),
    inst_nth_(lld::uart_to_nth(reinterpret_cast<uintptr_t>(p_inst_))),
    tx_dma_(_uart_to_tx_dma(inst_nth_)),
    rx_dma_(_uart_to_rx_dma(inst_nth_)){;}



void usart_enable_error_interrupt(void * p_inst_, const Enable en){
	USART_ITConfig(SDK_INST(p_inst_), USART_IT_PE, (en == EN));
	USART_ITConfig(SDK_INST(p_inst_), USART_IT_ERR, (en == EN));
}

void Uart::init(const Config & cfg){


    enable_rcc(EN);

    USART_Cmd(SDK_INST(p_inst_), DISABLE);

    set_remap(cfg.remap);


    if(cfg.tx_strategy != CommStrategy::Disabled){
        auto tx_pin = uart_to_tx_pin(inst_nth_, cfg.remap);
        tx_pin.afpp();
    }

    if(cfg.rx_strategy != CommStrategy::Disabled){
        auto rx_pin = uart_to_rx_pin(inst_nth_, cfg.remap);

        //串口的物理层是低有效
        rx_pin.inpu();
        // rx_pin.inflt();
    }

    const auto baudrate_hz = [&] -> uint32_t{
        const auto & baudrate = cfg.baudrate;
        if(baudrate.is<hal::NearestFreq>()){
            return baudrate.unwrap_as<hal::NearestFreq>().count;
        }else{
            __builtin_trap();
        }
    }();

    const USART_InitTypeDef USART_InitStructure{
        .USART_BaudRate = baudrate_hz,
        .USART_WordLength = USART_WordLength_8b,
        .USART_StopBits = USART_StopBits_1,
        .USART_Parity = USART_Parity_No,
        .USART_Mode =   uint16_t(
            uint16_t((cfg.tx_strategy != CommStrategy::Disabled) ? uint16_t(USART_Mode_Tx) : uint16_t(0)) |
            uint16_t((cfg.rx_strategy != CommStrategy::Disabled) ? uint16_t(USART_Mode_Rx) : uint16_t(0))),
        .USART_HardwareFlowControl = USART_HardwareFlowControl_None
    };

    while ((RAL_INST(p_inst_)->STATR.TXE) == RESET);
    RAL_INST(p_inst_)->DATAR.DR = 0xff;

    USART_Init(SDK_INST(p_inst_), &USART_InitStructure);

    usart_enable_error_interrupt(p_inst_, EN);

    register_nvic(EN);

    USART_Cmd(SDK_INST(p_inst_), ENABLE);

    // while ((RAL_INST(p_inst_)->STATR.RXNE) == RESET);
    // RAL_INST(p_inst_)->DATAR.DR;

    set_tx_strategy(cfg.tx_strategy);
    set_rx_strategy(cfg.rx_strategy);

}

size_t Uart::try_write_bytes(std::span<const uint8_t> bytes){
    switch(tx_strategy_){
        case CommStrategy::Blocking:{
            SDK_INST(p_inst_)->DATAR;

            //阻塞地发送字符
            for(size_t i = 0; i < bytes.size(); ++i){
                SDK_INST(p_inst_)->DATAR = bytes[i];
                while((SDK_INST(p_inst_)->STATR & USART_FLAG_TXE) == RESET);
            }
            while((SDK_INST(p_inst_)->STATR & USART_FLAG_TC) == RESET);
            return bytes.size();
        }
            break;
        case CommStrategy::Interrupt:{
            const auto written_len = tx_queue_.try_push(bytes);
            enable_tx_interrupt(EN);
            return written_len;
        }
        case CommStrategy::Dma:{
            const auto written_len = tx_queue_.try_push(bytes);
            return written_len;
        }
        case CommStrategy::Disabled:
            //运行到这里说明你可能配置错串口了 让未启用输出的串口输出数据
            __builtin_trap();
            return 0;
    }
    //无法到达这个控制流
    __builtin_unreachable();
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
    USART_HalfDuplexCmd(SDK_INST(p_inst_), (en == EN));
}


void Uart::register_nvic(const Enable en){
    UART_INTERRUPT_NVIC_PRIORITY.with_irqn(
        lld::uart_calc_nvic_irqn(inst_nth_)
    ).enable(EN);
}

void Uart::set_tx_strategy(const CommStrategy tx_strategy){
    if(tx_strategy_ == tx_strategy) return;

    switch(tx_strategy){
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
            enable_tx_interrupt(DISEN);
            break;
        default:
            __builtin_unreachable();
            break;
    }

    tx_strategy_ = tx_strategy;

}

void Uart::enable_tx_dma(const Enable en){
    USART_DMACmd(SDK_INST(p_inst_), USART_DMAReq_Tx, (en == EN));

    if(en != EN){
        tx_dma_.set_event_callback(nullptr);
        return;
    }

    tx_dma_.init({
        .mode = DmaMode::BurstMemoryToPeriphCircular,
        .priority = TX_DMA_DMA_PRIORITY
    });


    USART_SendData(SDK_INST(p_inst_), 0);
    tx_dma_.set_event_callback(
        [this](const DmaEvent ev){
            auto & self = *this;
            switch(ev){
            case DmaEvent::TransferComplete:
                //将数据从当前索引填充至末尾
                (void)self.tx_queue_.try_pop(std::span(
                    &tx_dma_buf_[tx_dma_buf_index_],
                    UART_TX_DMA_BUF_SIZE - tx_dma_buf_index_
                ));
                tx_dma_buf_index_ = 0;
                break;
            case DmaEvent::HalfTransfer:

                //将数据从当前索引填充至半满
                (void)self.tx_queue_.try_pop(std::span(
                    &tx_dma_buf_[tx_dma_buf_index_],
                    (UART_TX_DMA_BUF_SIZE / 2) - tx_dma_buf_index_
                ));
                tx_dma_buf_index_ = UART_TX_DMA_BUF_SIZE / 2;
                break;
            default:
                break;
            }
        }
    );
    tx_dma_.register_nvic(UART_TX_DMA_INTERRUPT_NVIC_PRIORITY, EN);
    tx_dma_.enable_interrupt<DmaIT::Done>(EN);
    tx_dma_.enable_interrupt<DmaIT::Half>(EN);

    tx_dma_.start_transfer_mem2pph<uint8_t>(
        (&SDK_INST(p_inst_)->DATAR),
        tx_dma_buf_.data(),
        UART_TX_DMA_BUF_SIZE
    );
}



void Uart::set_rx_strategy(const CommStrategy rx_strategy){
    if(rx_strategy_ == rx_strategy) return;

    switch(rx_strategy){
        case CommStrategy::Blocking:
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
            break;
        default:
            __builtin_unreachable();
            break;
    }
    rx_strategy_ = rx_strategy;

}


void Uart::enable_rx_dma(const Enable en){

    USART_DMACmd(SDK_INST(p_inst_), USART_DMAReq_Rx, (en == EN));
    if(en == DISEN){
        rx_dma_.set_event_callback(nullptr);
        return;
    }

    rx_dma_.init({
        .mode = DmaMode::PeriphToBurstMemoryCircular,
        .priority = RX_DMA_DMA_PRIORITY
    });


    rx_dma_.set_event_callback(
        [this](const DmaEvent ev) -> void{
            auto & self = *this;
            switch(ev){
            case DmaEvent::TransferComplete:{
                {
                    const auto uev = Event::RxBulk;
                    TRY_EMIT_EVENT(self, uev);
                }
                const size_t req_len = UART_RX_DMA_BUF_SIZE - rx_dma_buf_index_;
                //传送结束 将后半部分的pingpong区填入fifo中
                const size_t act_len = self.rx_queue_.try_push(std::span(
                    &rx_dma_buf_[rx_dma_buf_index_],
                    req_len
                ));

                if(act_len < req_len){
                    // 接收的数据没有被及时读取 接收队列无法继续存数据
                    RAISE_ABORT_ERR("uart rx(dma tc) queue full")
                }

                rx_dma_buf_index_ = 0;
            }
                break;
            case DmaEvent::HalfTransfer:{
                {
                    const auto uev = Event::RxBulk;
                    TRY_EMIT_EVENT(self, uev);
                }
                //传送进行一半 将前半部分的pingpong区填入fifo中
                const size_t req_len = HALF_UART_RX_DMA_BUF_SIZE - rx_dma_buf_index_;
                const size_t act_len = self.rx_queue_.try_push(std::span(
                    &rx_dma_buf_[rx_dma_buf_index_],
                    req_len
                ));

                if(act_len < req_len){
                    // 接收的数据没有被及时读取 接收队列无法继续存数据
                    RAISE_ABORT_ERR("uart rx(dma hc) queue full")
                }

                rx_dma_buf_index_ = HALF_UART_RX_DMA_BUF_SIZE;
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

    rx_dma_.start_transfer_pph2mem<uint8_t>(
        rx_dma_buf_.data(),
        &SDK_INST(p_inst_)->DATAR,
        UART_RX_DMA_BUF_SIZE
    );
}

void UartInterruptDispatcher::isr_rxne(Uart & self){
    switch(self.rx_strategy_){
        case CommStrategy::Dma:{
            break;
        }
        case CommStrategy::Interrupt:{
            const auto data = static_cast<uint8_t>(SDK_INST(self.p_inst_)->DATAR);
            if(const auto len = self.rx_queue_.try_push(data);
                len == 0
            ){
                // 接收的数据没有被及时读取 接收队列无法继续存数据
                RAISE_ABORT_ERR("uart rx(int) queue full")
            }
        }
            break;
        default:
            break;
    }

}

void UartInterruptDispatcher::isr_tc(Uart & self){
}

void UartInterruptDispatcher::isr_txe(Uart & self){
    switch(self.tx_strategy_){
        case CommStrategy::Dma:
            break;
        case CommStrategy::Interrupt:{
            uint8_t byte = 0;
            if(const auto len = self.tx_queue_.try_pop(byte);
                len != 0){
                SDK_INST(self.p_inst_)->DATAR = byte;
            }
        }
            break;
        default:
            break;
    }
}

void UartInterruptDispatcher::isr_rxidle(Uart & self){
    switch(self.rx_strategy_){
        case CommStrategy::Dma:{
            const size_t next_index = UART_RX_DMA_BUF_SIZE - self.rx_dma_.pending_count();

            if(next_index >= UART_RX_DMA_BUF_SIZE) [[unlikely]]
                __builtin_trap();

            #if 0
            // if((next_index != (UART_RX_DMA_BUF_SIZE / 2)) and (next_index != UART_RX_DMA_BUF_SIZE))
            #else
            if((next_index & (HALF_UART_RX_DMA_BUF_SIZE - 1)) != 0){
            #endif
                const auto req_len = size_t(next_index - self.rx_dma_buf_index_);
                const auto act_len = self.rx_queue_.try_push(std::span(
                    self.rx_dma_buf_.data() + self.rx_dma_buf_index_, req_len
                ));
                if(act_len != req_len){
                    // 接收的数据没有被及时读取 接收队列无法继续存数据
                    RAISE_ABORT_ERR("uart rx(dma) queue full")
                }
            }

            self.rx_dma_buf_index_ = next_index;
            {
                const auto ev = Event::RxIdle;
                TRY_EMIT_EVENT(self, ev);
            }
        };
            break;

        default:
            break;
    }
}

[[nodiscard]] size_t sink_bytes(std::span<const uint8_t> bytes){
    // TODO
    return 0;
}

void Uart::enable_rxne_interrupt(const Enable en){
    USART_ClearITPendingBit(SDK_INST(p_inst_), USART_IT_RXNE);
    USART_ITConfig(SDK_INST(p_inst_), USART_IT_RXNE, (en == EN));
}

void Uart::enable_tx_interrupt(const Enable en){
    USART_ITConfig(SDK_INST(p_inst_), USART_IT_TXE, (en == EN));
}


void Uart::enable_idle_interrupt(const Enable en){
    USART_ClearITPendingBit(SDK_INST(p_inst_), USART_IT_IDLE);
    USART_ITConfig(SDK_INST(p_inst_), USART_IT_IDLE, (en == EN));
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

void UartInterruptDispatcher::on_interrupt(Uart & self){
    auto * ral_inst = RAL_INST(self.p_inst_);
    const auto flags = ral_inst->get_flags();
    if(flags.any_fault()){
        if(flags.ORE){
            // 过载错误标志
            {
                //TODO
            }
            //这个事件无法自然消退
        }
        if(flags.FE){
            //帧错误
            {
                //TODO
            }

            ral_inst->STATR;
            ral_inst->DATAR;
        }
        if(flags.PE){
            //奇偶校验位错误
            {
                //TODO
            }

            ral_inst->STATR;
            ral_inst->DATAR;
        }
        if(flags.NE){
            // 噪声错误标志
            {
                //TODO
            }

            ral_inst->STATR;
            ral_inst->DATAR;
        }
    }

    if(flags.RXNE){
        // 对数据寄存器的读操作可以将该位清零
        UartInterruptDispatcher::isr_rxne(self);
    }

    if(flags.TXE){
        // 对数据寄存器进行写操作，此位将会清零
        UartInterruptDispatcher::isr_txe(self);
    }

    if(flags.TC){
        UartInterruptDispatcher::isr_tc(self);
        USART_ClearITPendingBit(SDK_INST(self.p_inst_), USART_IT_TC);
    }

    if(flags.IDLE){
        UartInterruptDispatcher::isr_rxidle(self);
        ral_inst->STATR;
        ral_inst->DATAR;
    }

};


