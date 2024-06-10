#include "uarthw.hpp"
#include "uarts.hpp"
#include "src/system.hpp"

static constexpr size_t rx_dma_buf_size = 64;
static constexpr size_t tx_dma_buf_size = 64;

#ifdef HAVE_UART2
UartHw uart2{USART2, UART2_TX_DMA_CH, UART2_RX_DMA_CH};
static size_t uart2_rx_dma_buf_index = 0;
static char uart2_rx_dma_buf[rx_dma_buf_size];
static char uart2_tx_dma_buf[tx_dma_buf_size];

__interrupt
void USART2_IRQHandler(void){
    if(USART_GetITStatus(USART2,USART_IT_RXNE)){
        uart2.rxBuf.addData(USART_ReceiveData(USART2));
        USART_ClearITPendingBit(USART2,USART_IT_RXNE);
    }else if(USART_GetITStatus(USART2,USART_IT_IDLE)){
        size_t index = rx_dma_buf_size - DMA1_Channel6->CNTR;
        if(index != rx_dma_buf_size / 2 && index != rx_dma_buf_size){
            for(size_t i = uart2_rx_dma_buf_index; i < index; i++) uart2.rxBuf.addData(uart2_rx_dma_buf[i]); 
        }
        uart2_rx_dma_buf_index = index;
        USART2->STATR;
        USART2->DATAR;//clear idle flag
    }else if(USART_GetITStatus(USART2,USART_IT_TXE)){
        if(uart2.txBuf.available()){
            USART2->DATAR = uart2.txBuf.getData();
        }else{
            USART_ITConfig(USART2, USART_IT_TXE, false);
        }
        USART_ClearITPendingBit(USART2,USART_IT_TXE);
    }else if(USART_GetFlagStatus(USART2,USART_FLAG_ORE)){
        USART_ReceiveData(USART2);
        USART_ClearFlag(USART2,USART_FLAG_ORE);
    }
}

//uart2 tx
__interrupt void DMA1_Channel7_IRQHandler(void){
    if(DMA_GetFlagStatus(DMA1_IT_TC7)){
        uart2.invokeTxDma();
        DMA_ClearFlag(DMA1_IT_TC7);
    }
}


//uart2 rx
__interrupt void DMA1_Channel6_IRQHandler(void){
    if(DMA_GetFlagStatus(DMA1_IT_TC6)){
        uart2.invokeRxDma();
        for(size_t i = uart2_rx_dma_buf_index; i < rx_dma_buf_size; i++) uart2.rxBuf.addData(uart2_rx_dma_buf[i]); 
        uart2_rx_dma_buf_index = 0;
        DMA_ClearFlag(DMA1_IT_TC6);
    }else if(DMA_GetFlagStatus(DMA1_IT_HT6)){
        for(size_t i = uart2_rx_dma_buf_index; i < rx_dma_buf_size / 2; i++) uart2.rxBuf.addData(uart2_rx_dma_buf[i]); 
        uart2_rx_dma_buf_index = rx_dma_buf_size / 2;
        DMA_ClearFlag(DMA1_IT_HT6); 
    }
}

#endif



void UartHw::enableRcc(const bool en){
    switch((uint32_t)instance){
        #ifdef HAVE_UART1
        case USART1_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, en);
            GPIO_PinRemapConfig(UART1_REMAP, UART1_REMAP_ENABLE);
            break;
        #endif
        #ifdef HAVE_UART2
        case USART2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, en);
            GPIO_PinRemapConfig(UART2_REMAP, UART2_REMAP_ENABLE);
            break;
        #endif
        #ifdef HAVE_UART3
        case USART3_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, en);
            break;
        #endif
        #ifdef HAVE_UART4
        case UART4_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, en);
            GPIO_PinRemapConfig(UART4_REMAP, UART4_REMAP_ENABLE);
            break;
        #endif
        #ifdef HAVE_UART5
        case UART5_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, en);
            break;
        #endif
        #ifdef HAVE_UART7
        case UART7_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7, en);
            GPIO_PinRemapConfig(UART7_REMAP, UART7_REMAP_ENABLE);
            break;
        #endif
        default:
            break;
    }
}


Gpio & UartHw::getRxPin(){
    Port * gpio_instance = nullptr;
    uint16_t gpio_pin = 0;

    switch((uint32_t)instance){
        #ifdef HAVE_UART1
        case USART1_BASE:
            gpio_instance = &UART1_TX_Port;
            gpio_pin = UART1_RX_Pin;
            break;
        #endif
        #ifdef HAVE_UART2
        case USART2_BASE:
            gpio_instance = &UART2_RX_Port;
            gpio_pin = UART2_RX_Pin;
            break;
        #endif
        #ifdef HAVE_UART3
        case USART3_BASE:
            gpio_instance = UART3_RX_Port;
            gpio_pin = UART3_RX_Pin;
            break;
        #endif
        #ifdef HAVE_UART4
        case UART4_BASE:
            gpio_instance = &UART4_RX_Port;
            gpio_pin = UART4_RX_Pin;
            break;
        #endif
        #ifdef HAVE_UART5
        case UART5_BASE:
            gpio_instance = UART5_RX_Port;
            gpio_pin = UART5_RX_Pin;
            break;
        #endif
        #ifdef HAVE_UART7
        case UART7_BASE:
            gpio_instance = &UART7_RX_Port;
            gpio_pin = UART7_RX_Pin;
            break;
        #endif
        default:
            return portA[Pin::None];
    }

    return (*gpio_instance)[(Pin)gpio_pin];
}

Gpio & UartHw::getTxPin(){
    Port * gpio_instance = nullptr;
    uint16_t gpio_pin = 0;

    switch((uint32_t)instance){
        #ifdef HAVE_UART1
        case USART1_BASE:
                gpio_instance = &portB;
                gpio_pin = UART1_TX_Pin;
                break;
        #endif
        #ifdef HAVE_UART2
        case USART2_BASE:
                gpio_instance = &UART2_TX_Port;
                gpio_pin = UART2_TX_Pin;
                break;
        #endif
        #ifdef HAVE_UART3
        case USART3_BASE:
                gpio_instance = UART2_TX_Port;
                gpio_pin = UART2_TX_Pin;
                break;
        #endif
        #ifdef HAVE_UART4
        case UART4_BASE:
            gpio_instance = &UART4_TX_Port;
            gpio_pin = UART4_TX_Pin;
            break;
        #endif
        #ifdef HAVE_UART7
        case UART7_BASE:
                gpio_instance = &UART7_TX_Port;
                gpio_pin = UART7_TX_Pin;
                break;
        #endif
        default:
            break;
    }

    return (*gpio_instance)[(Pin)gpio_pin];
}

void UartHw::enableIt(const bool en){
    uint8_t irqch;
    uint8_t pp;
    uint8_t sp;

    switch((uint32_t)instance){
        #ifdef HAVE_UART1
        case USART1_BASE:
            irqch = USART1_IRQn;
            pp = UART1_IT_PP;
            sp = UART1_IT_SP;
            break;
        #endif
        #ifdef HAVE_UART2
        case USART2_BASE:
            irqch = USART2_IRQn;
            pp = UART2_IT_PP;
            sp = UART2_IT_SP;
            break;
        #endif
        #ifdef HAVE_UART3
        case USART3_BASE:
            irqch = USART3_IRQn;
            pp = UART3_IT_PP;
            sp = UART3_IT_SP;
            break;
        #endif
        #ifdef HAVE_UART4
        case UART4_BASE:
            irqch = UART4_IRQn;
            pp = UART4_IT_PP;
            sp = UART4_IT_SP;
            break;
        #endif
        #ifdef HAVE_UART7
        case UART7_BASE:
            irqch = UART7_IRQn;
            pp = UART7_IT_PP;
            sp = UART7_IT_SP;

            break;
        #endif
        default:
            return;
    }

    NvicRequest(IRQn(irqch),pp, sp).enable();
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
    }
}

void UartHw::enableRxDma(const bool en){
    USART_DMACmd(instance, USART_DMAReq_Rx, en);
    if(en){
        rxDma.init(DmaChannel::Mode::toMemCircular);
        rxDma.enableIt({1,1});
        rxDma.enableDoneIt();
        rxDma.enableHalfIt();
        rxDma.begin((void *)uart2_rx_dma_buf, (void *)(&instance->DATAR), rx_dma_buf_size);
    }
}

void UartHw::invokeTxDma(){
    if(txDma.pending() == 0 && txBuf.available() != 0){
        size_t uart2_tx_amount = 0;
        while(txBuf.available()){
            uart2_tx_dma_buf[uart2_tx_amount++] = txBuf.getData();
            if(uart2_tx_amount >= tx_dma_buf_size){
                break;
            }
        }
        txDma.begin((void *)(&instance->DATAR), (void *)uart2_tx_dma_buf, uart2_tx_amount);
    }
    // if(txBuf.available() != 0){
    //     uart2_tx_amount = txBuf.straight();

    //     auto base = txBuf.read_ptr;
    //     for(size_t i = 0; i < uart2_tx_amount; i++){
    //         instance->DATAR = base[i];
    //         while((instance->STATR & USART_FLAG_TXE) == RESET);
    //     }

    //     txBuf.vent(uart2_tx_amount);
    //     invokeTxDma();
    // }

}

void UartHw::invokeRxDma(){
    rxDma.begin();
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
        txMethod = _txMethod;
        if(txMethod != CommMethod::None){
            Gpio & tx_pin = getTxPin();
            tx_pin.OutAfPP();
        }
        switch(txMethod){
            case CommMethod::Blocking:
                break;
            case CommMethod::Interrupt:
                enableTxDma(false);
                break;
            case CommMethod::Dma:
                enableTxDma();
                break;
            default:
                break;
        }
    }
}



void UartHw::setRxMethod(const CommMethod _rxMethod){
    if(rxMethod != _rxMethod){
        rxMethod = _rxMethod;
        if(rxMethod != CommMethod::None){
            Gpio & rx_pin = getRxPin();
            rx_pin.InPullUP();
        }
        switch(rxMethod){
            case CommMethod::Blocking:
                break;
            case CommMethod::Interrupt:
                enableRxDma(false);
                enableIdleIt(false);
                enableRxneIt();
                break;
            case CommMethod::Dma:
                enableRxneIt(false);
                enableIdleIt();
                enableRxDma();
                break;
            default:
                break;
        }
    }
}



void UartHw::init(const uint32_t baudRate, const CommMethod _rxMethod, const CommMethod _txMethod){
    enableRcc();

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = baudRate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = ((_txMethod != CommMethod::None) ? USART_Mode_Tx : 0) |
                                    ((_rxMethod != CommMethod::None) ? USART_Mode_Rx : 0);

    USART_Init(instance, &USART_InitStructure);

    enableIt(true);
    setTxMethod(_txMethod);
    setRxMethod(_rxMethod);
    USART_Cmd(instance, ENABLE);
}


void UartHw::_write(const char * data_ptr, const size_t & len){
    switch(txMethod){
        case CommMethod::Blocking:
            instance->DATAR;
            for(size_t i=0;i<len;i++)txBuf.addData(data_ptr[i]);
            while(txBuf.available()){
                instance->DATAR = txBuf.getData();
                while((instance->STATR & USART_FLAG_TXE) == RESET);
            }
            while((instance->STATR & USART_FLAG_TC) == RESET);
            break;
        case CommMethod::Interrupt:
            for(size_t i=0;i<len;i++)txBuf.addData(data_ptr[i]);
            invokeTxIt();

            break;
        case CommMethod::Dma:
            for(size_t i=0;i<len;i++)txBuf.addData(data_ptr[i]);
            invokeTxDma();
            break;
        default:
            break;
    }
}

void UartHw::_write(const char & data){
    switch(txMethod){
        case CommMethod::Blocking:
            txBuf.addData(data);

            instance->DATAR;
            instance->DATAR = txBuf.getData();
            while((instance->STATR & USART_FLAG_TC) == RESET);
            break;
        case CommMethod::Interrupt:
            txBuf.addData(data);
            invokeTxIt();
            break;
        case CommMethod::Dma:
            txBuf.addData(data);
            invokeTxDma();
            break;
        default:
            break;
    }
}


