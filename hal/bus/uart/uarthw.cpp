#include "core/platform.hpp"
#include "ral/ch32/ch32_common_uart_def.hpp"

#include "hal/dma/dma.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "uarthw.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::ral::CH32;


static Gpio map_uart_to_rxio(const void * inst){
    switch(reinterpret_cast<size_t>(inst)){
        #ifdef ENABLE_UART1
        case USART1_BASE:
            return UART1_RX_GPIO;
        #endif
        #ifdef ENABLE_UART2
        case USART2_BASE:
            return UART2_RX_GPIO;
        #endif
        #ifdef ENABLE_UART3
        case USART3_BASE:
            return UART3_RX_GPIO;
        #endif
        #ifdef ENABLE_UART4
        case UART4_BASE:
            return UART4_RX_GPIO;
        #endif
        #ifdef ENABLE_UART5
        case UART5_BASE:
            return UART5_RX_GPIO;
        #endif
        #ifdef ENABLE_UART6
        case UART6_BASE:
            return UART6_RX_GPIO;
        #endif
        #ifdef ENABLE_UART7
        case UART7_BASE:
            return UART7_RX_GPIO;
        #endif
        #ifdef ENABLE_UART8
        case UART8_BASE:
            return UART8_RX_GPIO;
        #endif
    }
    __builtin_trap();
}

static Gpio map_uart_to_txio(const void * inst){
    switch(reinterpret_cast<size_t>(inst)){
        #ifdef ENABLE_UART1
        case USART1_BASE:
            return UART1_TX_GPIO;
        #endif
        #ifdef ENABLE_UART2
        case USART2_BASE:
            return UART2_TX_GPIO;
        #endif
        #ifdef ENABLE_UART3
        case USART3_BASE:
            return UART3_TX_GPIO;
        #endif
        #ifdef ENABLE_UART4
        case UART4_BASE:
            return UART4_TX_GPIO;
        #endif
        #ifdef ENABLE_UART5
        case UART5_BASE:
            return UART5_TX_GPIO;
        #endif
        #ifdef ENABLE_UART6
        case UART6_BASE:
            return UART6_TX_GPIO;
        #endif
        #ifdef ENABLE_UART7
        case UART7_BASE:
            return UART7_TX_GPIO;
        #endif
        #ifdef ENABLE_UART8
        case UART8_BASE:
            return UART8_TX_GPIO;
        #endif
    }
    __builtin_trap();
}


Gpio UartHw::txio(){
    return map_uart_to_txio(inst_);
}
Gpio UartHw::rxio(){
    return map_uart_to_rxio(inst_);
}

void UartHw::enable_rcc(const Enable en){
    switch(reinterpret_cast<size_t>(inst_)){
        #ifdef ENABLE_UART1
        case USART1_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, en == EN);
            return;
        #endif
        #ifdef ENABLE_UART2
        case USART2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, en == EN);
            return;
        #endif
        #ifdef ENABLE_UART3
        case USART3_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, en == EN);
            return;
        #endif
        #ifdef ENABLE_UART4
        case UART4_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, en == EN);
            return;
        #endif
        #ifdef ENABLE_UART5
        case UART5_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, en == EN);
            return;
        #endif
        #ifdef ENABLE_UART6
        case UART6_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART6, en == EN);
            return;
        #endif
        #ifdef ENABLE_UART7
        case UART7_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7, en == EN);
            return;
        #endif
        #ifdef ENABLE_UART8
        case UART8_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, en == EN);
            return;
        #endif
    }
    __builtin_trap();
}

static constexpr uint8_t get_default_remap(const void * inst){
    switch(reinterpret_cast<size_t>(inst)){
        #ifdef ENABLE_UART1
        case USART1_BASE:
            return UART1_REMAP;
        #endif
        #ifdef ENABLE_UART2
        case USART2_BASE:
            return UART2_REMAP;
        #endif
        #ifdef ENABLE_UART3
        case USART3_BASE:
            return UART3_REMAP;
        #endif
        #ifdef ENABLE_UART4
        case UART4_BASE:
            return UART4_REMAP;
        #endif
        #ifdef ENABLE_UART5
        case UART5_BASE:
            return UART5_REMAP;
        #endif
        #ifdef ENABLE_UART6
        case UART6_BASE:
            return UART6_REMAP;
        #endif
        #ifdef ENABLE_UART7
        case UART7_BASE:
            return UART7_REMAP;
        #endif
        #ifdef ENABLE_UART8
        case UART8_BASE:
            return UART8_REMAP;
        #endif
    }
    __builtin_trap();
}

void UartHw::set_remap(const uint8_t remap){
    switch(reinterpret_cast<size_t>(inst_)){
        #ifdef ENABLE_UART1
        case USART1_BASE:
            switch(remap){
                case 0:
                    GPIO_PinRemapConfig(GPIO_Remap_USART1, DISABLE);
                    return;
                case 1:
                    GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
                    return;
                default:
                    break;
            }
        #endif
        #ifdef ENABLE_UART2
        case USART2_BASE:
            switch(remap){
                case 0:
                    GPIO_PinRemapConfig(GPIO_Remap_USART2, DISABLE);
                    return;
                case 1:
                    GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
                    return;
                default:
                    break;
            }
        #endif
        #ifdef ENABLE_UART3
        case USART3_BASE:
            switch (remap){
                case 0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART3, DISABLE);
                    return;
                case 1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap1_USART3, ENABLE);
                    return;
                case 2:
                    GPIO_PinRemapConfig(GPIO_PartialRemap2_USART3, ENABLE);
                    return;
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);
                    return;
                default:
                    break;
            }
            break;
        #endif
        #ifdef ENABLE_UART4
        case UART4_BASE:
            switch (remap){
                case 0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART4, DISABLE);
                    return;
                case 1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART4, ENABLE);
                    return;
                case 2:
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART4, ENABLE);
                    return;
                default:
                    break;
            }
            break;
        #endif
        #ifdef ENABLE_UART5
        case UART5_BASE:
            switch (remap){
                case 0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART5, DISABLE);
                    return;
                case 1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART5, ENABLE);
                    return;
                case 2:
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART5, ENABLE);
                    return;
                default:
                    break;
            }
            break;
        #endif
        #ifdef ENABLE_UART6
        case UART6_BASE:
            switch (remap){
                case 0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART6, DISABLE);
                    return;
                case 1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART6, ENABLE);
                    return;
                case 2:
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART6, ENABLE);
                    return;
                default:
                    break;
            }
            break;
        #endif
        #ifdef ENABLE_UART7
        case UART7_BASE:
            switch (remap){
                case 0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART7, DISABLE);
                    return;
                case 1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART7, ENABLE);
                    return;
                case 2:
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART7, ENABLE);
                    return;
                default:
                    break;
            }
            break;
        #endif
        #ifdef ENABLE_UART8
        case UART8_BASE:
            switch (remap){
                case 0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART8, DISABLE);
                    return;
                case 1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART8, ENABLE);
                    return;
                case 2:
                case 3:
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

static NvicRequest get_nvic_request(const void * inst){

    switch(reinterpret_cast<size_t>(inst)){
        #ifdef ENABLE_UART1
        case USART1_BASE:
            return NvicRequest(UART1_IT_PP, UART1_IT_SP, USART1_IRQn);
        #endif
        #ifdef ENABLE_UART2
        case USART2_BASE:
            return NvicRequest(UART2_IT_PP, UART2_IT_SP, USART2_IRQn);
        #endif
        #ifdef ENABLE_UART3
        case USART3_BASE:
            return NvicRequest(UART3_IT_PP, UART3_IT_SP, USART3_IRQn);
        #endif
        #ifdef ENABLE_UART4
        case UART4_BASE:
            return NvicRequest(UART4_IT_PP, UART4_IT_SP, UART4_IRQn);
        #endif
        #ifdef ENABLE_UART5
        case UART5_BASE:
            return NvicRequest(UART5_IT_PP, UART5_IT_SP, UART5_IRQn);
        #endif
        #ifdef ENABLE_UART6
        case UART6_BASE:
            return NvicRequest(UART6_IT_PP, UART6_IT_SP, UART6_IRQn);
        #endif
        #ifdef ENABLE_UART7
        case UART7_BASE:
            return NvicRequest(UART8_IT_PP, UART7_IT_SP, UART7_IRQn);
        #endif
        #ifdef ENABLE_UART8
        case UART8_BASE:
            return NvicRequest(UART8_IT_PP, UART8_IT_SP, UART8_IRQn);
        #endif
    }
    __builtin_trap();
}

void UartHw::register_nvic(const Enable en){
    get_nvic_request(inst_).enable(EN);
}


void UartHw::on_rxidle_interrupt(){
    switch(rx_strategy_){
        case CommStrategy::Dma:{
            // __builtin_trap();
            const size_t next_index = UART_RX_DMA_BUF_SIZE - rx_dma_.remaining();

            if(unlikely(next_index >= UART_RX_DMA_BUF_SIZE)) 
                __builtin_trap();

            if((next_index != (UART_RX_DMA_BUF_SIZE / 2)) and (next_index != UART_RX_DMA_BUF_SIZE)){
            // if(true){
                (void)this->rx_fifo_.push(std::span(
                    &rx_dma_buf_[rx_dma_buf_index_], (next_index - rx_dma_buf_index_))); 
                // (void)this->rx_fifo_.push(1); 
            }

            rx_dma_buf_index_ = next_index;
            invoke_callback(Event::RxIdle);
        }; 
            break;

        default:
            __builtin_trap();
    }
}


void UartHw::on_rx_dma_done(){
    //将数据从当前索引填充至末尾
    (void)this->rx_fifo_.push(std::span(
        &rx_dma_buf_[rx_dma_buf_index_], 
        UART_RX_DMA_BUF_SIZE - rx_dma_buf_index_
    )); 
    rx_dma_buf_index_ = 0;
}

void UartHw::on_rx_dma_half(){

    //将数据从当前索引填充至半满
    (void)this->rx_fifo_.push(std::span(
        &rx_dma_buf_[rx_dma_buf_index_], 
        (UART_RX_DMA_BUF_SIZE / 2) - rx_dma_buf_index_
    )); 
    rx_dma_buf_index_ = UART_RX_DMA_BUF_SIZE / 2;
}


void UartHw::enable_single_line_mode(const Enable en){
    USART_HalfDuplexCmd(inst_, en == EN);
    if(en == EN){
        txio().inpu();
    }else{
        txio().outod();
    }
}

void UartHw::invoke_tx_dma(){
    if(tx_dma_.remaining())    return;

    // 如果发送队列为空，则说明发送完成
    if(tx_fifo_.available() == 0){
        invoke_callback(Event::TxIdle);
        return;
    }
    const size_t tx_amount = tx_fifo_.available();
    (void)tx_fifo_.pop(std::span(tx_dma_buf_.data(), tx_amount));
    tx_dma_.start_transfer_mem2pph<char>(
        (&inst_->DATAR), 
        tx_dma_buf_.data(), tx_amount
    );
}



void UartHw::set_tx_strategy(const CommStrategy tx_strategy){
    if(tx_strategy_ == tx_strategy) return;

    auto tx_gpio = txio();
    if(tx_strategy != CommStrategy::Nil){
        tx_gpio.afpp();
    }else{
        // tx_gpio.inflt();
    }

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
        
    auto rx_gpio = rxio();

    if(rx_strategy != CommStrategy::Nil){
        rx_gpio.inpu();
    }

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
    set_remap(get_default_remap(inst_));

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

    USART_Init(inst_, &USART_InitStructure);
    USART_Cmd(inst_, ENABLE);

    register_nvic(EN);
    set_tx_strategy(cfg.tx_strategy);
    set_rx_strategy(cfg.rx_strategy);
}

void UartHw::writeN(const char * pbuf, const size_t len){
    switch(tx_strategy_){
        case CommStrategy::Blocking:
            inst_->DATAR;

            (void)tx_fifo_.push(std::span(pbuf, len));
            while(tx_fifo_.available()){
                inst_->DATAR = tx_fifo_.pop();
                while((inst_->STATR & USART_FLAG_TXE) == RESET);
            }
            while((inst_->STATR & USART_FLAG_TC) == RESET);
            
            break;
        case CommStrategy::Interrupt:
            (void)tx_fifo_.push(std::span(pbuf, len));
            enable_tx_it(EN);

            break;
        case CommStrategy::Dma:
            (void)tx_fifo_.push(std::span(pbuf, len));
            invoke_tx_dma();
            break;
        default:
            break;
    }
}

void UartHw::write1(const char data){
    switch(tx_strategy_){
        case CommStrategy::Blocking:
            tx_fifo_.push(data);

            inst_->DATAR;
            inst_->DATAR = tx_fifo_.pop();
            while((inst_->STATR & USART_FLAG_TC) == RESET);
            break;

        case CommStrategy::Interrupt:
            tx_fifo_.push(data);
            enable_tx_it(EN);
            break;

        case CommStrategy::Dma:
            tx_fifo_.push(data);
            invoke_tx_dma();
            break;
        default:
            break;
    }
}



void UartHw::enable_tx_dma(const Enable en){
    USART_DMACmd(inst_, USART_DMAReq_Tx, en == EN);

    if(en == EN){
        static constexpr NvicPriority NVIC_PRIORITY = {1,1};
        tx_dma_.init({DmaMode::ToPeriph, DmaPriority::Medium});

        tx_dma_.register_nvic(NVIC_PRIORITY, EN);
        tx_dma_.enable_interrupt<DmaIT::Done>(EN);
        tx_dma_.set_event_handler(
            [this](const DmaEvent event){
                switch(event){
                    case DmaEvent::TransferComplete:
                        this->invoke_tx_dma();
                        break;
                    default:
                        break;
                }
            }
        );
    }else{
        tx_dma_.set_event_handler(nullptr);
    }
}

void UartHw::enable_rx_dma(const Enable en){
    USART_DMACmd(inst_, USART_DMAReq_Rx, en == EN);
    if(en == EN){
        static constexpr NvicPriority NVIC_PRIORITY = {1,1};
        rx_dma_.init({DmaMode::ToMemCircular, DmaPriority::Medium});

        rx_dma_.register_nvic(NVIC_PRIORITY, EN);
        rx_dma_.enable_interrupt<DmaIT::Done>(EN);
        rx_dma_.enable_interrupt<DmaIT::Half>(EN);
        rx_dma_.set_event_handler(

            [this](const DmaEvent event) -> void{
                switch(event){
                    case DmaEvent::TransferComplete:
                        this->on_rx_dma_done();
                        break;
                    case DmaEvent::HalfTransfer:
                        this->on_rx_dma_half();
                        break;
                    default:
                        break;
                }
            }
        );

        rx_dma_.start_transfer_pph2mem<char>(
            rx_dma_buf_.data(), 
            &inst_->DATAR, 
            UART_RX_DMA_BUF_SIZE
        );
    }else{
        rx_dma_.set_event_handler(nullptr);
    }
}

void UartHw::enable_rxne_it(const Enable en){
    USART_ClearITPendingBit(inst_, USART_IT_RXNE);
    USART_ITConfig(inst_, USART_IT_RXNE, en == EN);
}

void UartHw::enable_tx_it(const Enable en){
    USART_ITConfig(inst_, USART_IT_TXE, en == EN);
}


void UartHw::enable_idle_it(const Enable en){
    USART_ClearITPendingBit(inst_, USART_IT_IDLE);
    USART_ITConfig(inst_, USART_IT_IDLE, en == EN);
}


namespace ymd::hal{
#ifdef ENABLE_UART1
UartHw uart1{USART1, UART1_TX_DMA_CH, UART1_RX_DMA_CH};
#endif

#ifdef ENABLE_UART2
UartHw uart2{USART2, UART2_TX_DMA_CH, UART2_RX_DMA_CH};
#endif

#ifdef ENABLE_UART3
UartHw uart3{USART3, UART3_TX_DMA_CH, UART3_RX_DMA_CH};
#endif

#ifdef ENABLE_UART4
UartHw uart4{UART4, UART4_TX_DMA_CH, UART4_RX_DMA_CH};
#endif

#ifdef ENABLE_UART5
UartHw uart5{UART5, UART5_TX_DMA_CH, UART5_RX_DMA_CH};
#endif

#ifdef ENABLE_UART6
UartHw uart6{UART6, UART6_TX_DMA_CH, UART6_RX_DMA_CH};
#endif

#ifdef ENABLE_UART7
UartHw uart7{UART7, UART7_TX_DMA_CH, UART7_RX_DMA_CH};
#endif

#ifdef ENABLE_UART8
UartHw uart8{UART8, UART8_TX_DMA_CH, UART8_RX_DMA_CH};
#endif
}

#define UART_IT_TEMPLATE(name, uname, fname)\
__interrupt void fname(void){\
    const auto events = uname##_Inst->get_events();\
    if(events.RXNE){\
        name.on_rxne_interrupt();\
        uname##_Inst->clear_events({.RXNE = 1});\
    }else if(events.IDLE){\
        name.on_rxidle_interrupt();\
        uname##_Inst->STATR;\
        uname##_Inst->DATAR;\
    }else if(events.TXE){\
        name.on_txe_interrupt();\
        uname##_Inst->clear_events({.TXE = 1});\
    }else if(events.ORE){\
        uname##_Inst->DATAR;\
        uname##_Inst->clear_events({.ORE = 1});\
    }\
}\

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#ifdef ENABLE_UART1
UART_IT_TEMPLATE(uart1, USART1, USART1_IRQHandler)
#endif

#ifdef ENABLE_UART2
UART_IT_TEMPLATE(uart2, USART2, USART2_IRQHandler)
#endif

#ifdef ENABLE_UART3
UART_IT_TEMPLATE(uart3, USART3, USART3_IRQHandler)
#endif

#ifdef ENABLE_UART4
UART_IT_TEMPLATE(uart4, USART4, UART4_IRQHandler)
#endif

#ifdef ENABLE_UART5
UART_IT_TEMPLATE(uart5, USART5, UART5_IRQHandler)
#endif

#ifdef ENABLE_UART6
UART_IT_TEMPLATE(uart6, USART6, UART6_IRQHandler)
#endif

#ifdef ENABLE_UART7
UART_IT_TEMPLATE(uart7, USART7, UART7_IRQHandler)
#endif

#ifdef ENABLE_UART8
UART_IT_TEMPLATE(uart8, USART8, UART8_IRQHandler)
#endif


#pragma GCC diagnostic pop
