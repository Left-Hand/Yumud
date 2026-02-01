#include "uart.hpp"
#include "uart_layout.hpp"
#include "ral/uart.hpp"

#include "hal/dma/dma.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/sdk.hpp"


using namespace ymd;
using namespace ymd::hal;

using Event = Uart::Event;

#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SDK_INST(x) (reinterpret_cast<COPY_CONST(x, USART_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::USART_Def)>(x))

#define EMIT_EVENT(x)   if(self.event_callback_ != nullptr) {self.event_callback_(x);}

namespace {
[[maybe_unused]] static constexpr Nth _uart_to_nth(const uint32_t base_addr){
    switch(base_addr){
        #ifdef USART1_PRESENT
        case USART1_BASE:
            return Nth(1);
        #endif
        #ifdef USART2_PRESENT
        case USART2_BASE:
            return Nth(2);
        #endif
        #ifdef USART3_PRESENT
        case USART3_BASE:
            return Nth(3);
        #endif
        #ifdef UART4_PRESENT
        case UART4_BASE:
            return Nth(4);
        #endif
        #ifdef UART5_PRESENT
        case UART5_BASE:
            return Nth(5);
        #endif
        #ifdef UART6_PRESENT
        case UART6_BASE:
            return Nth(6);
        #endif
        #ifdef UART7_PRESENT
        case UART7_BASE:
            return Nth(7);
        #endif
        #ifdef UART8_PRESENT
        case UART8_BASE:
            return Nth(8);
        #endif
    }
    __builtin_trap();
}


template<UartRemap REMAP>
[[maybe_unused]] static Gpio _uart_to_tx_pin(const uint32_t base_addr){
    const auto nth = _uart_to_nth(base_addr);
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
[[maybe_unused]] static Gpio _uart_to_rx_pin(const uint32_t base_addr){
    const auto nth = _uart_to_nth(base_addr);
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
[[maybe_unused]] static Gpio uart_to_##name##_pin(const uint32_t base_addr, const UartRemap remap){\
    switch(remap){\
        case UartRemap::_0: return _uart_to_##name##_pin<UartRemap::_0>(base_addr);\
        case UartRemap::_1: return _uart_to_##name##_pin<UartRemap::_1>(base_addr);\
        case UartRemap::_2: return _uart_to_##name##_pin<UartRemap::_2>(base_addr);\
        case UartRemap::_3: return _uart_to_##name##_pin<UartRemap::_3>(base_addr);\
    }\
    __builtin_trap();\
}\

DEF_UART_BIND_PIN_LAYOUTER(tx)
DEF_UART_BIND_PIN_LAYOUTER(rx)

static DmaChannel & uart_to_rx_dma(const void * inst){
    switch(reinterpret_cast<size_t>(inst)){
        #ifdef USART1_PRESENT
        case USART1_BASE:
            return USART1_RX_DMA_CH;
        #endif
        #ifdef USART2_PRESENT
        case USART2_BASE:
            return USART2_RX_DMA_CH;
        #endif
        #ifdef USART3_PRESENT
        case USART3_BASE:
            return USART3_RX_DMA_CH;
        #endif
        #ifdef UART4_PRESENT
        case UART4_BASE:
            return UART4_RX_DMA_CH;
        #endif
        #ifdef UART5_PRESENT
        case UART5_BASE:
            return UART5_RX_DMA_CH;
        #endif
        #ifdef UART6_PRESENT
        case UART6_BASE:
            return UART6_RX_DMA_CH;
        #endif
        #ifdef UART7_PRESENT
        case UART7_BASE:
            return UART7_RX_DMA_CH;
        #endif
        #ifdef UART8_PRESENT
        case UART8_BASE:
            return UART8_RX_DMA_CH;
        #endif
    }
    __builtin_trap();

}
static DmaChannel & uart_to_tx_dma(const void * inst){
    switch(reinterpret_cast<size_t>(inst)){
        #ifdef USART1_PRESENT
        case USART1_BASE:
            return USART1_TX_DMA_CH;
        #endif
        #ifdef USART2_PRESENT
        case USART2_BASE:
            return USART2_TX_DMA_CH;
        #endif
        #ifdef USART3_PRESENT
        case USART3_BASE:
            return USART3_TX_DMA_CH;
        #endif
        #ifdef UART4_PRESENT
        case UART4_BASE:
            return UART4_TX_DMA_CH;
        #endif
        #ifdef UART5_PRESENT
        case UART5_BASE:
            return UART5_TX_DMA_CH;
        #endif
        #ifdef UART6_PRESENT
        case UART6_BASE:
            return UART6_TX_DMA_CH;
        #endif
        #ifdef UART7_PRESENT
        case UART7_BASE:
            return UART7_TX_DMA_CH;
        #endif
        #ifdef UART8_PRESENT
        case UART8_BASE:
            return UART8_TX_DMA_CH;
        #endif
    }
    __builtin_trap();
}

static IRQn get_nvic_irqn(const void * inst){

    switch(reinterpret_cast<size_t>(inst)){
        #ifdef USART1_PRESENT
        case USART1_BASE:
            return USART1_IRQn;
        #endif
        #ifdef USART2_PRESENT
        case USART2_BASE:
            return USART2_IRQn;
        #endif
        #ifdef USART3_PRESENT
        case USART3_BASE:
            return USART3_IRQn;
        #endif
        #ifdef UART4_PRESENT
        case UART4_BASE:
            return UART4_IRQn;
        #endif
        #ifdef UART5_PRESENT
        case UART5_BASE:
            return UART5_IRQn;
        #endif
        #ifdef UART6_PRESENT
        case UART6_BASE:
            return UART6_IRQn;
        #endif
        #ifdef UART7_PRESENT
        case UART7_BASE:
            return UART7_IRQn;
        #endif
        #ifdef UART8_PRESENT
        case UART8_BASE:
            return UART8_IRQn;
        #endif
    }
    __builtin_trap();
}

static void uart_enable_rcc(const void * inst, const Enable en){
    switch(reinterpret_cast<size_t>(inst)){
        #ifdef USART1_PRESENT
        case USART1_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, en == EN);
            return;
        #endif
        #ifdef USART2_PRESENT
        case USART2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, en == EN);
            return;
        #endif
        #ifdef USART3_PRESENT
        case USART3_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, en == EN);
            return;
        #endif
        #ifdef UART4_PRESENT
        case UART4_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, en == EN);
            return;
        #endif
        #ifdef UART5_PRESENT
        case UART5_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, en == EN);
            return;
        #endif
        #ifdef UART6_PRESENT
        case UART6_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART6, en == EN);
            return;
        #endif
        #ifdef UART7_PRESENT
        case UART7_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7, en == EN);
            return;
        #endif
        #ifdef UART8_PRESENT
        case UART8_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, en == EN);
            return;
        #endif
    }
    __builtin_trap();
}

static void uart_set_remap(const void * inst, const UartRemap remap){
    switch(reinterpret_cast<size_t>(inst)){
        #ifdef USART1_PRESENT
        case USART1_BASE:
            switch(remap){
                case UartRemap::_0:
                    GPIO_PinRemapConfig(GPIO_Remap_USART1, DISABLE);
                    return;
                case UartRemap::_1:
                    GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
                    return;
                default:
                    break;
            }
        #endif
        #ifdef USART2_PRESENT
        case USART2_BASE:
            switch(remap){
                case UartRemap::_0:
                    GPIO_PinRemapConfig(GPIO_Remap_USART2, DISABLE);
                    return;
                case UartRemap::_1:
                    GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
                    return;
                default:
                    break;
            }
        #endif
        #ifdef USART3_PRESENT
        case USART3_BASE:
            switch (remap){
                case UartRemap::_0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART3, DISABLE);
                    return;
                case UartRemap::_1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap1_USART3, ENABLE);
                    return;
                case UartRemap::_2:
                    GPIO_PinRemapConfig(GPIO_PartialRemap2_USART3, ENABLE);
                    return;
                case UartRemap::_3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);
                    return;
                default:
                    break;
            }
            break;
        #endif
        #ifdef UART4_PRESENT
        case UART4_BASE:
            switch (remap){
                case UartRemap::_0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART4, DISABLE);
                    return;
                case UartRemap::_1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART4, ENABLE);
                    return;
                case UartRemap::_2:
                case UartRemap::_3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART4, ENABLE);
                    return;
            }
            break;
        #endif
        #ifdef UART5_PRESENT
        case UART5_BASE:
            switch (remap){
                case UartRemap::_0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART5, DISABLE);
                    return;
                case UartRemap::_1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART5, ENABLE);
                    return;
                case UartRemap::_2:
                case UartRemap::_3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART5, ENABLE);
                    return;
                default:
                    break;
            }
            break;
        #endif
        #ifdef UART6_PRESENT
        case UART6_BASE:
            switch (remap){
                case UartRemap::_0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART6, DISABLE);
                    return;
                case UartRemap::_1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART6, ENABLE);
                    return;
                case UartRemap::_2:
                case UartRemap::_3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART6, ENABLE);
                    return;
                default:
                    break;
            }
            break;
        #endif
        #ifdef UART7_PRESENT
        case UART7_BASE:
            switch (remap){
                case UartRemap::_0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART7, DISABLE);
                    return;
                case UartRemap::_1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART7, ENABLE);
                    return;
                case UartRemap::_2:
                case UartRemap::_3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART7, ENABLE);
                    return;
                default:
                    break;
            }
            break;
        #endif
        #ifdef UART8_PRESENT
        case UART8_BASE:
            switch (remap){
                case UartRemap::_0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART8, DISABLE);
                    return;
                case UartRemap::_1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART8, ENABLE);
                    return;
                case UartRemap::_2:
                case UartRemap::_3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART8, ENABLE);
                    return;
                default:
                    break;
            }
            break;
        #endif
    }
    __builtin_trap();
}

}

static constexpr const NvicPriority UART_RX_DMA_NVIC_PRIORITY = {1,4};
static constexpr const NvicPriority UART_TX_DMA_NVIC_PRIORITY = {1,5};
static constexpr const NvicPriority UART_INTERRUPT_NVIC_PRIORITY = {1,3};

static constexpr const DmaPriority RX_DMA_DMA_PRIORITY = DmaPriority::Medium;
static constexpr const DmaPriority TX_DMA_DMA_PRIORITY = DmaPriority::Medium;


static_assert(std::has_single_bit(UART_RX_DMA_BUF_SIZE)); //缓冲区大小必须是2的幂
static_assert(std::has_single_bit(UART_TX_DMA_BUF_SIZE)); //缓冲区大小必须是2的幂

static constexpr size_t HALF_UART_RX_DMA_BUF_SIZE = UART_RX_DMA_BUF_SIZE / 2;

Uart::Uart(
    void * inst
):
    inst_(inst),
    tx_dma_(uart_to_tx_dma(inst)),
    rx_dma_(uart_to_rx_dma(inst)){;}

void usart_enable_error_interrupt(void * inst_, const Enable en){
	USART_ITConfig(SDK_INST(inst_), USART_IT_PE, en == EN);
	USART_ITConfig(SDK_INST(inst_), USART_IT_ERR, en == EN);
}

void Uart::init(const Config & cfg){
    enable_rcc(EN);

    USART_Cmd(SDK_INST(inst_), DISABLE);

    set_remap(cfg.remap);


    if(cfg.tx_strategy != CommStrategy::Nil){
        auto tx_pin = uart_to_tx_pin(reinterpret_cast<uint32_t>(inst_), cfg.remap);
        tx_pin.afpp();
    }

    if(cfg.rx_strategy != CommStrategy::Nil){
        auto rx_pin = uart_to_rx_pin(reinterpret_cast<uint32_t>(inst_), cfg.remap);

        //串口的物理层是低有效
        rx_pin.inpu();
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
            uint16_t((cfg.tx_strategy != CommStrategy::Nil) ? uint16_t(USART_Mode_Tx) : uint16_t(0)) |
            uint16_t((cfg.rx_strategy != CommStrategy::Nil) ? uint16_t(USART_Mode_Rx) : uint16_t(0))),
        .USART_HardwareFlowControl = USART_HardwareFlowControl_None
    };

    USART_Init(SDK_INST(inst_), &USART_InitStructure);

    usart_enable_error_interrupt(inst_, EN);

    register_nvic(EN);

    set_tx_strategy(cfg.tx_strategy);
    set_rx_strategy(cfg.rx_strategy);

    USART_Cmd(SDK_INST(inst_), ENABLE);

}

size_t Uart::try_write_bytes(std::span<const uint8_t> bytes){
    switch(tx_strategy_){
        case CommStrategy::Blocking:{
            SDK_INST(inst_)->DATAR;

            //阻塞地发送字符
            for(size_t i = 0; i < bytes.size(); ++i){
                SDK_INST(inst_)->DATAR = bytes[i];
                while((SDK_INST(inst_)->STATR & USART_FLAG_TXE) == RESET);
            }
            while((SDK_INST(inst_)->STATR & USART_FLAG_TC) == RESET);
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
        case CommStrategy::Nil:
            //运行到这里说明你可能配置错串口了 让未启用输出的串口输出数据
            __builtin_trap();
            return 0;
    }
    //无法到达这个控制流
    __builtin_trap();
}

size_t Uart::try_write_byte(const uint8_t byte){
    return try_write_bytes(std::span(&byte, 1));
}



void Uart::deinit(){
    enable_rcc(DISEN);
}

void Uart::enable_rcc(const Enable en){
    uart_enable_rcc(inst_, en);
}


void Uart::set_remap(const UartRemap remap){
    uart_set_remap(inst_, remap);
}

void Uart::enable_single_line_mode(const Enable en){
    USART_HalfDuplexCmd(SDK_INST(inst_), en == EN);
}


void Uart::register_nvic(const Enable en){
    UART_INTERRUPT_NVIC_PRIORITY.with_irqn(get_nvic_irqn(SDK_INST(inst_))).enable(EN);
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
    USART_DMACmd(SDK_INST(inst_), USART_DMAReq_Tx, en == EN);

    if(en != EN){
        tx_dma_.set_event_callback(nullptr);
        return;
    }

    tx_dma_.init({
        .mode = DmaMode::BurstMemoryToPeriphCircular,
        .priority = TX_DMA_DMA_PRIORITY
    });


    USART_SendData(SDK_INST(inst_), 0);
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
    tx_dma_.register_nvic(UART_TX_DMA_NVIC_PRIORITY, EN);
    tx_dma_.enable_interrupt<DmaIT::Done>(EN);
    tx_dma_.enable_interrupt<DmaIT::Half>(EN);

    tx_dma_.start_transfer_mem2pph<uint8_t>(
        (&SDK_INST(inst_)->DATAR),
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

    USART_DMACmd(SDK_INST(inst_), USART_DMAReq_Rx, en == EN);
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
                    EMIT_EVENT(uev);
                }
                const size_t req_len = UART_RX_DMA_BUF_SIZE - rx_dma_buf_index_;
                //传送结束 将后半部分的pingpong区填入fifo中
                const size_t act_len = self.rx_queue_.try_push(std::span(
                    &rx_dma_buf_[rx_dma_buf_index_],
                    req_len
                ));

                if(act_len < req_len){
                    //TODO
                    // 接收的数据没有被及时读取 接收fifo无法继续存数据
                    __builtin_trap();
                }

                rx_dma_buf_index_ = 0;
            }
                break;
            case DmaEvent::HalfTransfer:{
                {
                    const auto uev = Event::RxBulk;
                    EMIT_EVENT(uev);
                }
                //传送进行一半 将前半部分的pingpong区填入fifo中
                const size_t req_len = HALF_UART_RX_DMA_BUF_SIZE - rx_dma_buf_index_;
                const size_t act_len = self.rx_queue_.try_push(std::span(
                    &rx_dma_buf_[rx_dma_buf_index_],
                    req_len
                ));
                if(act_len < req_len){
                    //TODO
                    // 接收的数据没有被及时读取 接收fifo无法继续存数据
                    __builtin_trap();
                }

                rx_dma_buf_index_ = HALF_UART_RX_DMA_BUF_SIZE;
            }
                break;
            default:
                break;
            }
        }
    );

    rx_dma_.register_nvic(UART_RX_DMA_NVIC_PRIORITY, EN);
    rx_dma_.enable_interrupt<DmaIT::Done>(EN);
    rx_dma_.enable_interrupt<DmaIT::Half>(EN);

    rx_dma_.start_transfer_pph2mem<uint8_t>(
        rx_dma_buf_.data(),
        &SDK_INST(inst_)->DATAR,
        UART_RX_DMA_BUF_SIZE
    );
}

void UartInterruptDispatcher::isr_rxne(Uart & self){
    switch(self.rx_strategy_){
        case CommStrategy::Interrupt:{
            const auto data = uint8_t(SDK_INST(self.inst_)->DATAR);
            if(const auto len = self.rx_queue_.try_push(data);
                len == 0
            ){
                //TODO
                // 接收的数据没有被及时读取 接收fifo无法继续存数据
                __builtin_trap();
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
                SDK_INST(self.inst_)->DATAR = byte;
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
                    //TODO
                    // 接收的数据没有被及时读取 接收fifo无法继续存数据
                    __builtin_trap();
                }
            }

            self.rx_dma_buf_index_ = next_index;
            {
                const auto ev = Event::RxIdle;
                EMIT_EVENT(ev);
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
    USART_ClearITPendingBit(SDK_INST(inst_), USART_IT_RXNE);
    USART_ITConfig(SDK_INST(inst_), USART_IT_RXNE, en == EN);
}

void Uart::enable_tx_interrupt(const Enable en){
    USART_ITConfig(SDK_INST(inst_), USART_IT_TXE, en == EN);
}


void Uart::enable_idle_interrupt(const Enable en){
    USART_ClearITPendingBit(SDK_INST(inst_), USART_IT_IDLE);
    USART_ITConfig(SDK_INST(inst_), USART_IT_IDLE, en == EN);
}


void UartInterruptDispatcher::on_interrupt(Uart & self){
    auto * ral_inst = RAL_INST(self.inst_);
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
        USART_ClearITPendingBit(SDK_INST(self.inst_), USART_IT_TC);
    }

    if(flags.IDLE){
        UartInterruptDispatcher::isr_rxidle(self);
        ral_inst->STATR;
        ral_inst->DATAR;
    }

};


