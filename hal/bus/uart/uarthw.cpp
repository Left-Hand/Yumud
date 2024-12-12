#include "sys/core/platform.h"

#include "uarthw.hpp"

using namespace ymd;

#define UART_TX_DMA_BUF_SIZE UART_DMA_BUF_SIZE
#define UART_RX_DMA_BUF_SIZE UART_DMA_BUF_SIZE

#define UART_IT_TEMPLATE(name, uname)\
__interrupt void uname##_IRQHandler(void){\
    if(USART_GetITStatus(uname,USART_IT_RXNE)){\
        name.rxneHandle();\
        USART_ClearITPendingBit(uname,USART_IT_RXNE);\
    }else if(USART_GetITStatus(uname,USART_IT_IDLE)){\
        name.idleHandle();\
        uname->STATR;\
        uname->DATAR;\
    }else if(USART_GetITStatus(uname,USART_IT_TXE)){\
        name.txeHandle();\
        USART_ClearITPendingBit(uname,USART_IT_TXE);\
    }else if(USART_GetFlagStatus(uname,USART_FLAG_ORE)){\
        USART_ReceiveData(uname);\
        USART_ClearFlag(uname,USART_FLAG_ORE);\
    }\
}\


#ifdef ENABLE_UART1
UART_IT_TEMPLATE(uart1, USART1)
UartHw uart1{USART1, UART1_TX_DMA_CH, UART1_RX_DMA_CH};
#endif


#ifdef ENABLE_UART2
UART_IT_TEMPLATE(uart2, USART2)
UartHw uart2{USART2, UART2_TX_DMA_CH, UART2_RX_DMA_CH};
#endif

#ifdef ENABLE_UART3
UART_IT_TEMPLATE(uart3, USART3)
UartHw uart3{USART3, UART3_TX_DMA_CH, UART3_RX_DMA_CH};
#endif


#ifdef ENABLE_UART4
UART_IT_TEMPLATE(uart4, UART4)
UartHw uart4{UART4, UART4_TX_DMA_CH, UART4_RX_DMA_CH};
#endif

#ifdef ENABLE_UART5
UART_IT_TEMPLATE(uart5, UART5)
UartHw uart5{UART5, UART5_TX_DMA_CH, UART5_RX_DMA_CH};
#endif


#ifdef ENABLE_UART6
UART_IT_TEMPLATE(uart6, UART6)
UartHw uart6{UART6, UART6_TX_DMA_CH, UART6_RX_DMA_CH};
#endif

#ifdef ENABLE_UART7
UART_IT_TEMPLATE(uart7, UART7)
UartHw uart7{UART7, UART7_TX_DMA_CH, UART7_RX_DMA_CH};
#endif


#ifdef ENABLE_UART8
UART_IT_TEMPLATE(uart8, UART8)
UartHw uart8{UART8, UART8_TX_DMA_CH, UART8_RX_DMA_CH};
#endif


void UartHw::enableRcc(const bool en){
    switch((uint32_t)instance){
        #ifdef ENABLE_UART1
        case USART1_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, en);
            if(UART1_REMAP){
                GPIO_PinRemapConfig(GPIO_Remap_USART1, true);
            }
            break;
        #endif
        #ifdef ENABLE_UART2
        case USART2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, en);
            if(UART2_REMAP){
                GPIO_PinRemapConfig(GPIO_Remap_USART2, true);
            }
            break;
        #endif
        #ifdef ENABLE_UART3
        case USART3_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, en);
            switch (UART3_REMAP){
                case 1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap1_USART3, ENABLE);
                    break;
                case 2:
                    GPIO_PinRemapConfig(GPIO_PartialRemap2_USART3, ENABLE);
                    break;
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);
                    break;
                default:
                    break;
            }
            break;
        #endif
        #ifdef ENABLE_UART4
        case UART4_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, en);
            switch (UART4_REMAP){
                case 1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART4, ENABLE);
                    break;
                case 2:
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART4, ENABLE);
                    break;
                default:
                    break;
            }
            break;
        #endif
        #ifdef ENABLE_UART5
        case UART5_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, en);
            switch (UART4_REMAP){
                case 1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART5, ENABLE);
                    break;
                case 2:
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART5, ENABLE);
                    break;
                default:
                    break;
            }
            break;
        #endif
        #ifdef ENABLE_UART6
        case UART6_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART6, en);
            switch (UART6_REMAP){
                case 1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART6, ENABLE);
                    break;
                case 2:
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART6, ENABLE);
                    break;
                default:
                    break;
            }
            break;
        #endif
        #ifdef ENABLE_UART7
        case UART7_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7, en);
            
            switch (UART7_REMAP){
                case 1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART7, ENABLE);
                    break;
                case 2:
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART7, ENABLE);
                    break;
                default:
                    break;
            }
            break;
        #endif
        #ifdef ENABLE_UART8
        case UART8_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, en);
            switch (UART8_REMAP){
                case 1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART8, ENABLE);
                    break;
                case 2:
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART8, ENABLE);
                    break;
                default:
                    break;
            }
            break;
        #endif
        default:
            break;
    }
}


void UartHw::rxneHandle(){
    this->rx_fifo.push(USART_ReceiveData(instance));
}
void UartHw::txeHandle(){

}

void UartHw::idleHandle(){
    if(rxMethod == CommMethod::Dma){
        size_t index = UART_RX_DMA_BUF_SIZE - rxDma.pending();
        if(index != UART_RX_DMA_BUF_SIZE / 2 && index != UART_RX_DMA_BUF_SIZE){
            // for(size_t i = rx_dma_buf_index; i < index; i++) this->rx_fifo.push(rx_dma_buf[i]); 
            this->rx_fifo.push(&rx_dma_buf[rx_dma_buf_index], (index - rx_dma_buf_index)); 
        }
        rx_dma_buf_index = index;
        EXECUTE(rxPostCb);
    }
}

Gpio & UartHw::rxio(){
    switch((uint32_t)instance){
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
        default:
            return GpioNull;
    }
}

Gpio & UartHw::txio(){
    switch((uint32_t)instance){
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
        default:
            return GpioNull;
    }
}

void UartHw::enableIt(const bool en){
    IRQn irq = IRQn::Software_IRQn;
    uint8_t pp = 1;
    uint8_t sp = 7;

    switch((uint32_t)instance){
        #ifdef ENABLE_UART1
        case USART1_BASE:
            irq = USART1_IRQn;
            pp = UART1_IT_PP;
            sp = UART1_IT_SP;
            break;
        #endif
        #ifdef ENABLE_UART2
        case USART2_BASE:
            irq = USART2_IRQn;
            pp = UART2_IT_PP;
            sp = UART2_IT_SP;
            break;
        #endif
        #ifdef ENABLE_UART3
        case USART3_BASE:
            irq = USART3_IRQn;
            pp = UART3_IT_PP;
            sp = UART3_IT_SP;
            break;
        #endif
        #ifdef ENABLE_UART4
        case UART4_BASE:
            irq = UART4_IRQn;
            pp = UART4_IT_PP;
            sp = UART4_IT_SP;
            break;
        #endif
        #ifdef ENABLE_UART5
        case UART5_BASE:
            irq = UART5_IRQn;
            pp = UART5_IT_PP;
            sp = UART5_IT_SP;
            break;
        #endif
        #ifdef ENABLE_UART6
        case UART6_BASE:
            irq = UART6_IRQn;
            pp = UART6_IT_PP;
            sp = UART6_IT_SP;
            break;
        #endif
        #ifdef ENABLE_UART7
        case UART7_BASE:
            irq = UART7_IRQn;
            pp = UART7_IT_PP;
            sp = UART7_IT_SP;
            break;
        #endif
        #ifdef ENABLE_UART8
        case UART8_BASE:
            irq = UART8_IRQn;
            pp = UART8_IT_PP;
            sp = UART8_IT_SP;
            break;
        #endif
        default:
            return;
    }

    NvicRequest(pp, sp, irq).enable();
}

void UartHw::invokeTxIt(){
    USART_ITConfig(instance, USART_IT_TXE, ENABLE);
}

void UartHw::enableTxDma(const bool en){
    USART_DMACmd(instance, USART_DMAReq_Tx, en);
    if(en){
        txDma.init(DmaChannel::Mode::toPeriph);
        txDma.enableIt({1,1});
        txDma.enableDoneIt();
        txDma.configDataBytes(1);
        txDma.bindDoneCb([this](){this->invokeTxDma();});
    }
}

void UartHw::rxDmaDoneHandler(){
    //将数据从当前索引填充至末尾
    rxDma.start();
    // for(size_t i = rx_dma_buf_index; i < UART_RX_DMA_BUF_SIZE; i++) this->rx_fifo.push(rx_dma_buf[i]); 
    this->rx_fifo.push(&rx_dma_buf[rx_dma_buf_index], UART_RX_DMA_BUF_SIZE - rx_dma_buf_index); 
    rx_dma_buf_index = 0;
}

void UartHw::rxDmaHalfHandler(){
    //将数据从当前索引填充至半满
    // for(size_t i = rx_dma_buf_index; i < UART_RX_DMA_BUF_SIZE / 2; i++) this->rx_fifo.push(rx_dma_buf[i]); 
    this->rx_fifo.push(&rx_dma_buf[rx_dma_buf_index], (UART_RX_DMA_BUF_SIZE / 2) - rx_dma_buf_index); 
    rx_dma_buf_index = UART_RX_DMA_BUF_SIZE / 2;
}

void UartHw::enableRxDma(const bool en){
    USART_DMACmd(instance, USART_DMAReq_Rx, en);
    if(en){
        rxDma.init(DmaChannel::Mode::toMemCircular);
        rxDma.enableIt({1,1});
        rxDma.enableDoneIt();
        rxDma.enableHalfIt();
        rxDma.configDataBytes(1);

        rxDma.bindDoneCb(std::bind(&UartHw::rxDmaDoneHandler, this));
        rxDma.bindHalfCb(std::bind(&UartHw::rxDmaHalfHandler, this));
        rxDma.start((void *)rx_dma_buf.begin(), (const void *)(size_t)(&instance->DATAR), UART_RX_DMA_BUF_SIZE);
    }else{
        rxDma.bindDoneCb(nullptr);
        rxDma.bindHalfCb(nullptr);
    }
}

void UartHw::invokeTxDma(){
    if(txDma.pending() == 0){
        if(tx_fifo.available()){
            const size_t tx_amount = MIN(tx_fifo.available(), tx_dma_buf.size());
            tx_fifo.pop(tx_dma_buf.begin(), tx_amount);
            txDma.start((void *)(size_t)(&instance->DATAR), (const void *)tx_dma_buf.begin(), tx_amount);
        }else{
            EXECUTE(txPostCb);
        }
    }
}


void UartHw::enableRxneIt(const bool en){
    USART_ClearITPendingBit(instance, USART_IT_RXNE);
    USART_ITConfig(instance, USART_IT_RXNE, en);
}

void UartHw::enableIdleIt(const bool en){
    USART_ClearITPendingBit(instance, USART_IT_IDLE);
    USART_ITConfig(instance, USART_IT_IDLE, en);
}

void UartHw::setTxMethod(const CommMethod _txMethod){
    if(txMethod != _txMethod){

        Gpio & tx_pin = txio();
        if(txMethod != CommMethod::None){
            tx_pin.afpp();
        }else{
            // tx_pin.inflt();
        }

        switch(txMethod){
            case CommMethod::Blocking:
                break;
            case CommMethod::Interrupt:
                enableTxDma(false);
                break;
            case CommMethod::Dma:
                enableTxDma(true);
                break;
            default:
                break;
        }

        txMethod = _txMethod;
    }
}



void UartHw::setRxMethod(const CommMethod _rxMethod){
    if(rxMethod != _rxMethod){
        
        Gpio & rx_pin = rxio();
        if(rxMethod != CommMethod::None){
            rx_pin.inpu();
        }else{
            // rx_pin.inflt();
        }
        switch(rxMethod){
            case CommMethod::Blocking:
                break;
            case CommMethod::Interrupt:
                enableRxDma(false);
                enableIdleIt(false);
                enableRxneIt(true);
                break;
            case CommMethod::Dma:
                enableRxneIt(false);
                enableIdleIt(true);
                enableRxDma(true);
                break;
            default:
                break;
        }
        rxMethod = _rxMethod;
    }
}



void UartHw::init(const uint32_t baudRate, const CommMethod _txMethod, const CommMethod _rxMethod){
    enableRcc(true);

    USART_InitTypeDef USART_InitStructure{
        .USART_BaudRate = baudRate,
        .USART_WordLength = USART_WordLength_8b,
        .USART_StopBits = USART_StopBits_1,
        .USART_Parity = USART_Parity_No,
        .USART_Mode =   uint16_t(((_txMethod != CommMethod::None) ? uint16_t(USART_Mode_Tx) : 0u) |
                        ((_rxMethod != CommMethod::None) ? uint16_t(USART_Mode_Rx) : 0u)),
        .USART_HardwareFlowControl = USART_HardwareFlowControl_None
    };

    USART_Init(instance, &USART_InitStructure);
    USART_Cmd(instance, ENABLE);

    enableIt(true);
    setTxMethod(_txMethod);
    setRxMethod(_rxMethod);
}

UartHw::Error UartHw::lead(const uint8_t index){
    while((instance->STATR & USART_FLAG_TXE) == RESET);
    return ErrorType::OK;
}

void UartHw::trail(){
    while((instance->STATR & USART_FLAG_TC) == RESET);
}

void UartHw::write(const char * data_ptr, const size_t len){
    switch(txMethod){
        case CommMethod::Blocking:
            instance->DATAR;
            // for(size_t i=0;i<len;i++)tx_fifo.push(data_ptr[i]);
            tx_fifo.push(data_ptr, len);
            while(tx_fifo.available()){
                instance->DATAR = tx_fifo.pop();
                while((instance->STATR & USART_FLAG_TXE) == RESET);
            }
            while((instance->STATR & USART_FLAG_TC) == RESET);
            break;
        case CommMethod::Interrupt:
            // for(size_t i=0;i<len;i++)tx_fifo.push(data_ptr[i]);
            tx_fifo.push(data_ptr, len);
            invokeTxIt();

            break;
        case CommMethod::Dma:
            // for(size_t i=0;i<len;i++)tx_fifo.push(data_ptr[i]);
            tx_fifo.push(data_ptr, len);
            invokeTxDma();
            break;
        default:
            break;
    }
}

void UartHw::write(const char data){
    switch(txMethod){
        case CommMethod::Blocking:
            tx_fifo.push(data);

            instance->DATAR;
            instance->DATAR = tx_fifo.pop();
            while((instance->STATR & USART_FLAG_TC) == RESET);
            break;
        case CommMethod::Interrupt:
            tx_fifo.push(data);
            invokeTxIt();
            break;
        case CommMethod::Dma:
            tx_fifo.push(data);
            invokeTxDma();
            break;
        default:
            break;
    }
}


