#include "core/platform.hpp"
#include "core/utils/Option.hpp"
#include "ral/uart.hpp"

#include "hal/dma/dma.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "uarthw.hpp"

#include "core/sdk.hpp"
#include "uart_layout.hpp"

using namespace ymd;
using namespace ymd::hal;


#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SDK_INST(x) (reinterpret_cast<COPY_CONST(x, USART_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::USART_Def)>(x))

[[maybe_unused]] static Nth _uart_to_nth(const void * inst){
    switch(reinterpret_cast<size_t>(inst)){
        #ifdef UART1_PRESENT
        case USART1_BASE:
            return Nth(1);
        #endif
        #ifdef UART2_PRESENT
        case USART2_BASE:
            return Nth(2);
        #endif
        #ifdef UART3_PRESENT
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
[[maybe_unused]] static Gpio _uart_to_tx_pin(const void * inst){
    const auto nth = _uart_to_nth(inst);
    switch(nth.count()){
        #ifdef UART1_PRESENT
        case 1:
            return pintag_to_pin<uart::tx_pin_t<1, REMAP>>();
        #endif
        #ifdef UART2_PRESENT
        case 2:
            return pintag_to_pin<uart::tx_pin_t<2, REMAP>>();
        #endif
        #ifdef UART3_PRESENT
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
[[maybe_unused]] static Gpio _uart_to_rx_pin(const void * inst){
    const auto nth = _uart_to_nth(inst);
    switch(nth.count()){
        #ifdef UART1_PRESENT
        case 1:
            return pintag_to_pin<uart::rx_pin_t<1, REMAP>>();
        #endif
        #ifdef UART2_PRESENT
        case 2:
            return pintag_to_pin<uart::rx_pin_t<2, REMAP>>();
        #endif
        #ifdef UART3_PRESENT
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
[[maybe_unused]] static Gpio uart_to_##name##_pin(const void * inst, const UartRemap remap){\
    switch(remap){\
        case UartRemap::_0: return _uart_to_##name##_pin<UartRemap::_0>(inst);\
        case UartRemap::_1: return _uart_to_##name##_pin<UartRemap::_1>(inst);\
        case UartRemap::_2: return _uart_to_##name##_pin<UartRemap::_2>(inst);\
        case UartRemap::_3: return _uart_to_##name##_pin<UartRemap::_3>(inst);\
    }\
    __builtin_trap();\
}\

DEF_UART_BIND_PIN_LAYOUTER(tx)
DEF_UART_BIND_PIN_LAYOUTER(rx)

static DmaChannel & uart_to_rx_dma(const void * inst){
    switch(reinterpret_cast<size_t>(inst)){
        #ifdef UART1_PRESENT
        case USART1_BASE:
            return UART1_RX_DMA_CH;
        #endif
        #ifdef UART2_PRESENT
        case USART2_BASE:
            return UART2_RX_DMA_CH;
        #endif
        #ifdef UART3_PRESENT
        case USART3_BASE:
            return UART3_RX_DMA_CH;
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
        #ifdef UART1_PRESENT
        case USART1_BASE:
            return UART1_TX_DMA_CH;
        #endif
        #ifdef UART2_PRESENT
        case USART2_BASE:
            return UART2_TX_DMA_CH;
        #endif
        #ifdef UART3_PRESENT
        case USART3_BASE:
            return UART3_TX_DMA_CH;
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
        #ifdef UART1_PRESENT
        case USART1_BASE:
            return USART1_IRQn;
        #endif
        #ifdef UART2_PRESENT
        case USART2_BASE:
            return USART2_IRQn;
        #endif
        #ifdef UART3_PRESENT
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
        #ifdef UART1_PRESENT
        case USART1_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, en == EN);
            return;
        #endif
        #ifdef UART2_PRESENT
        case USART2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, en == EN);
            return;
        #endif
        #ifdef UART3_PRESENT
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
        #ifdef UART1_PRESENT
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
        #ifdef UART2_PRESENT
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
        #ifdef UART3_PRESENT
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
                default:
                    break;
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


UartHw::UartHw(
    void * inst
):
    inst_(inst), 
    tx_dma_(uart_to_tx_dma(inst)), 
    rx_dma_(uart_to_rx_dma(inst)){;}

void UartHw::enable_rcc(const Enable en){
    uart_enable_rcc(SDK_INST(inst_), en);
}


void UartHw::set_remap(const UartRemap remap){
    uart_set_remap(SDK_INST(inst_), remap);
}


void UartHw::register_nvic(const Enable en){
    NvicPriority{1,1}.with_irqn(get_nvic_irqn(SDK_INST(inst_))).enable(EN);
}



void UartHw::enable_single_line_mode(const Enable en){
    USART_HalfDuplexCmd(SDK_INST(inst_), en == EN);
    // if(en == EN){
    //     uart_to_txio(inst_, remap).inpu();
    // }else{
    //     uart_to_txio(inst_, remap).outod();
    // }
}

void UartHw::invoke_tx_dma(){
    if(tx_dma_.remaining())    return;

    // 如果发送队列为空，则说明发送完成
    if(tx_fifo_.length() == 0){
        invoke_callback(Event::TxIdle);
        return;
    }
    const size_t num_tx_bytes = tx_fifo_.length();
    (void)tx_fifo_.try_pop(std::span(tx_dma_buf_.data(), num_tx_bytes));
    tx_dma_.start_transfer_mem2pph<char>(
        (&SDK_INST(inst_)->DATAR), 
        tx_dma_buf_.data(), num_tx_bytes
    );
}



void UartHw::set_tx_strategy(const CommStrategy tx_strategy){
    if(tx_strategy_ == tx_strategy) return;


    switch(tx_strategy){
        case CommStrategy::Blocking:
            break;
        case CommStrategy::Interrupt:
            enable_tx_dma(DISEN);
            break;
        case CommStrategy::Dma:
            enable_tx_dma(EN);
            break;
        default:
            break;
    }

    tx_strategy_ = tx_strategy;
    
}


void UartHw::set_rx_strategy(const CommStrategy rx_strategy){
    if(rx_strategy_ == rx_strategy) return;
        
    switch(rx_strategy){
        case CommStrategy::Blocking:
            break;
        case CommStrategy::Interrupt:
            enable_rx_dma(DISEN);
            enable_idle_it(DISEN);
            enable_rxne_it(EN);
            break;
        case CommStrategy::Dma:
            enable_rxne_it(DISEN);
            enable_idle_it(EN);
            enable_rx_dma(EN);
            break;
        default:
            break;
    }
    rx_strategy_ = rx_strategy;
    
}


void UartHw::init(const Config & cfg){
    enable_rcc(EN);
    set_remap(cfg.remap);

    const USART_InitTypeDef USART_InitStructure{
        .USART_BaudRate = cfg.baudrate,
        .USART_WordLength = USART_WordLength_8b,
        .USART_StopBits = USART_StopBits_1,
        .USART_Parity = USART_Parity_No,
        .USART_Mode =   uint16_t(
            uint16_t((cfg.tx_strategy != CommStrategy::Nil) ? uint16_t(USART_Mode_Tx) : uint16_t(0)) |
            uint16_t((cfg.rx_strategy != CommStrategy::Nil) ? uint16_t(USART_Mode_Rx) : uint16_t(0))),
        .USART_HardwareFlowControl = USART_HardwareFlowControl_None
    };

    USART_Init(SDK_INST(inst_), &USART_InitStructure);
    USART_Cmd(SDK_INST(inst_), ENABLE);

    register_nvic(EN);
    set_tx_strategy(cfg.tx_strategy);
    set_rx_strategy(cfg.rx_strategy);

    if(cfg.tx_strategy != CommStrategy::Nil){
        auto tx_pin = uart_to_tx_pin(inst_, cfg.remap);
        tx_pin.afpp();
    }
    
    if(cfg.rx_strategy != CommStrategy::Nil){
        auto rx_pin = uart_to_rx_pin(inst_, cfg.remap);
        rx_pin.inpu();
    }
}

size_t UartHw::try_write_chars(const char * pchars, const size_t len){
    switch(tx_strategy_){
        case CommStrategy::Blocking:{
            SDK_INST(inst_)->DATAR;

            // (void)tx_fifo_.push(std::span(pchars, len));
            for(size_t i = 0; i < len; ++i){
                SDK_INST(inst_)->DATAR = pchars[i];
                while((SDK_INST(inst_)->STATR & USART_FLAG_TXE) == RESET);
            }
            while((SDK_INST(inst_)->STATR & USART_FLAG_TC) == RESET);
            return len;
        }
            break;
        case CommStrategy::Interrupt:{
            const auto written_len = tx_fifo_.try_push(std::span(pchars, len));
            enable_tx_it(EN);
            return written_len;
        }
        case CommStrategy::Dma:{
            const auto written_len = tx_fifo_.try_push(std::span(pchars, len));
            invoke_tx_dma();
            return written_len;
        }
        default:
            //无法到达这个控制流
            __builtin_trap();
    }
}

size_t UartHw::try_write_char(const char chr){
    return try_write_chars(&chr, 1);
}



void UartHw::enable_tx_dma(const Enable en){
    USART_DMACmd(SDK_INST(inst_), USART_DMAReq_Tx, en == EN);

    if(en != EN){
        tx_dma_.set_event_handler(nullptr);
    }

    static constexpr NvicPriority NVIC_PRIORITY = {1,1};
    tx_dma_.init({
        .mode = DmaMode::ToPeriph, 
        .priority = DmaPriority::Medium
    });

    tx_dma_.register_nvic(NVIC_PRIORITY, EN);
    tx_dma_.enable_interrupt<DmaIT::Done>(EN);
    tx_dma_.set_event_handler(
        [this](const DmaEvent ev){
            switch(ev){
                case DmaEvent::TransferComplete:
                    this->invoke_tx_dma();
                    break;
                default:
                    break;
            }
        }
    );

}

void UartHw::enable_rx_dma(const Enable en){
    USART_DMACmd(SDK_INST(inst_), USART_DMAReq_Rx, en == EN);
    if(en == EN){
        static constexpr NvicPriority NVIC_PRIORITY = {1,1};
        rx_dma_.init({DmaMode::ToMemCircular, DmaPriority::Medium});

        rx_dma_.register_nvic(NVIC_PRIORITY, EN);
        rx_dma_.enable_interrupt<DmaIT::Done>(EN);
        rx_dma_.enable_interrupt<DmaIT::Half>(EN);
        rx_dma_.set_event_handler(
            [this](const DmaEvent ev) -> void{
                switch(ev){
                    case DmaEvent::TransferComplete:
                        //将数据从当前索引填充至末尾
                        (void)this->rx_fifo_.try_push(std::span(
                            &rx_dma_buf_[rx_dma_buf_index_], 
                            UART_RX_DMA_BUF_SIZE - rx_dma_buf_index_
                        )); 
                        rx_dma_buf_index_ = 0;
                        break;
                    case DmaEvent::HalfTransfer:

                        //将数据从当前索引填充至半满
                        (void)this->rx_fifo_.try_push(std::span(
                            &rx_dma_buf_[rx_dma_buf_index_], 
                            (UART_RX_DMA_BUF_SIZE / 2) - rx_dma_buf_index_
                        )); 
                        rx_dma_buf_index_ = UART_RX_DMA_BUF_SIZE / 2;
                        break;
                    default:
                        break;
                }
            }
        );

        rx_dma_.start_transfer_pph2mem<char>(
            rx_dma_buf_.data(), 
            &SDK_INST(inst_)->DATAR, 
            UART_RX_DMA_BUF_SIZE
        );
    }else{
        rx_dma_.set_event_handler(nullptr);
    }
}

void UartHw::on_rxne_interrupt(){
    (void)this->rx_fifo_.try_push(uint8_t(SDK_INST(inst_)->DATAR));
}

void UartHw::on_txe_interrupt(){

}

void UartHw::on_rxidle_interrupt(){
    switch(rx_strategy_){
        case CommStrategy::Dma:{
            const size_t next_index = UART_RX_DMA_BUF_SIZE - rx_dma_.remaining();

            if(next_index >= UART_RX_DMA_BUF_SIZE) [[unlikely]]
                __builtin_trap();

            if((next_index != (UART_RX_DMA_BUF_SIZE / 2)) and (next_index != UART_RX_DMA_BUF_SIZE)){
                (void)this->rx_fifo_.try_push(std::span(
                    &rx_dma_buf_[rx_dma_buf_index_], (next_index - rx_dma_buf_index_))); 
            }

            rx_dma_buf_index_ = next_index;
            invoke_callback(Event::RxIdle);
        }; 
            break;

        default:
            __builtin_trap();
    }
}


void UartHw::enable_rxne_it(const Enable en){
    USART_ClearITPendingBit(SDK_INST(inst_), USART_IT_RXNE);
    USART_ITConfig(SDK_INST(inst_), USART_IT_RXNE, en == EN);
}

void UartHw::enable_tx_it(const Enable en){
    USART_ITConfig(SDK_INST(inst_), USART_IT_TXE, en == EN);
}


void UartHw::enable_idle_it(const Enable en){
    USART_ClearITPendingBit(SDK_INST(inst_), USART_IT_IDLE);
    USART_ITConfig(SDK_INST(inst_), USART_IT_IDLE, en == EN);
}


namespace ymd::hal{
#ifdef UART1_PRESENT
UartHw uart1{USART1};
#endif

#ifdef UART2_PRESENT
UartHw uart2{USART2};
#endif

#ifdef UART3_PRESENT
UartHw uart3{USART3};
#endif

#ifdef UART4_PRESENT
UartHw uart4{UART4};
#endif

#ifdef UART5_PRESENT
UartHw uart5{UART5};
#endif

#ifdef UART6_PRESENT
UartHw uart6{UART6};
#endif

#ifdef UART7_PRESENT
UartHw uart7{UART7};
#endif

#ifdef UART8_PRESENT
UartHw uart8{UART8};
#endif
}

#define UART_IT_TEMPLATE(name, uname, fname)\
__interrupt void fname(void){\
    const auto events = ral::uname##_Inst->get_events();\
    if(events.RXNE){\
        name.on_rxne_interrupt();\
        ral::uname##_Inst->clear_events({.RXNE = 1});\
    }else if(events.IDLE){\
        name.on_rxidle_interrupt();\
        ral::uname##_Inst->STATR;\
        ral::uname##_Inst->DATAR;\
    }else if(events.TXE){\
        name.on_txe_interrupt();\
        ral::uname##_Inst->clear_events({.TXE = 1});\
    }else if(events.ORE){\
        ral::uname##_Inst->DATAR;\
        ral::uname##_Inst->clear_events({.ORE = 1});\
    }\
}\

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#ifdef UART1_PRESENT
UART_IT_TEMPLATE(uart1, USART1, USART1_IRQHandler)
#endif

#ifdef UART2_PRESENT
UART_IT_TEMPLATE(uart2, USART2, USART2_IRQHandler)
#endif

#ifdef UART3_PRESENT
UART_IT_TEMPLATE(uart3, USART3, USART3_IRQHandler)
#endif

#ifdef UART4_PRESENT
UART_IT_TEMPLATE(uart4, USART4, UART4_IRQHandler)
#endif

#ifdef UART5_PRESENT
UART_IT_TEMPLATE(uart5, USART5, UART5_IRQHandler)
#endif

#ifdef UART6_PRESENT
UART_IT_TEMPLATE(uart6, USART6, UART6_IRQHandler)
#endif

#ifdef UART7_PRESENT
UART_IT_TEMPLATE(uart7, USART7, UART7_IRQHandler)
#endif

#ifdef UART8_PRESENT
UART_IT_TEMPLATE(uart8, USART8, UART8_IRQHandler)
#endif


#pragma GCC diagnostic pop
