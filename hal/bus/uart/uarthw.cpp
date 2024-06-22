#include "uarthw.hpp"
#include "sys/system.hpp"

#define UART_CB_TEMPLATE(name)\
static UartHw::Callback name##_rxne_cb;\
static UartHw::Callback name##_txe_cb;\
static UartHw::Callback name##_idle_cb;\

#define UART_IT_TEMPLATE(name, uname)\
__interrupt void uname##_IRQHandler(void){\
    if(USART_GetITStatus(uname,USART_IT_RXNE)){\
        EXECUTE(name##_rxne_cb);\
        USART_ClearITPendingBit(uname,USART_IT_RXNE);\
    }else if(USART_GetITStatus(uname,USART_IT_IDLE)){\
        EXECUTE(name##_idle_cb);\
        uname->STATR;\
        uname->DATAR;\
    }else if(USART_GetITStatus(uname,USART_IT_TXE)){\
        EXECUTE(name##_txe_cb);\
        USART_ClearITPendingBit(uname,USART_IT_TXE);\
    }else if(USART_GetFlagStatus(uname,USART_FLAG_ORE)){\
        USART_ReceiveData(uname);\
        USART_ClearFlag(uname,USART_FLAG_ORE);\
    }\
}\

#define UART_TEMPLATE(name, pname, uname)\
UART_CB_TEMPLATE(name)\
UART_IT_TEMPLATE(name, (uname))\


// UartHw name{uname, pname##_TX_DMA_CH, pname##_RX_DMA_CH};


#ifdef HAVE_UART2
UART_CB_TEMPLATE(uart2)
UART_IT_TEMPLATE(uart2, USART2)
UartHw uart2{USART2, UART2_TX_DMA_CH, UART2_RX_DMA_CH};

#endif

#ifdef HAVE_UART1
UART_CB_TEMPLATE(uart1)
UART_IT_TEMPLATE(uart1, USART1)
UartHw uart1{USART1, UART1_TX_DMA_CH, UART1_RX_DMA_CH};

#endif


void UartHw::bindRxneCb(Callback && cb){
    switch((uint32_t)instance){
        #ifdef HAVE_UART1
        case USART1_BASE:
            uart1_rxne_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART2
        case USART2_BASE:
            uart2_rxne_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART3
        case USART3_BASE:
            uart3_rxne_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART4
        case UART4_BASE:
            uart4_rxne_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART5
        case UART5_BASE:
            uart5_rxne_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART6
        case UART6_BASE:
            uart6_rxne_cb = cb;
            break;
        #endif
    }
}

void UartHw::bindTxeCb(Callback && cb){
    switch((uint32_t)instance){
        #ifdef HAVE_UART1
        case USART1_BASE:
            uart1_txe_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART2
        case USART2_BASE:
            uart2_txe_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART3
        case USART3_BASE:
            uart3_txe_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART4
        case UART4_BASE:
            uart4_txe_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART5
        case UART5_BASE:
            uart5_txe_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART6
        case UART6_BASE:
            uart6_txe_cb = cb;
            break;
        #endif
    }
}

void UartHw::bindIdleCb(Callback && cb){
    switch((uint32_t)instance){
        #ifdef HAVE_UART1
        case USART1_BASE:
            uart1_idle_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART2
        case USART2_BASE:
            uart2_idle_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART3
        case USART3_BASE:
            uart3_idle_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART4
        case UART4_BASE:
            uart4_idle_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART5
        case UART5_BASE:
            uart5_idle_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART6
        case UART6_BASE:
            uart6_idle_cb = cb;
            break;
        #endif
    }
}

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
    IRQn irq;
    uint8_t pp;
    uint8_t sp;

    switch((uint32_t)instance){
        #ifdef HAVE_UART1
        case USART1_BASE:
            irq = USART1_IRQn;
            pp = UART1_IT_PP;
            sp = UART1_IT_SP;
            break;
        #endif
        #ifdef HAVE_UART2
        case USART2_BASE:
            irq = USART2_IRQn;
            pp = UART2_IT_PP;
            sp = UART2_IT_SP;
            break;
        #endif
        #ifdef HAVE_UART3
        case USART3_BASE:
            irq = USART3_IRQn;
            pp = UART3_IT_PP;
            sp = UART3_IT_SP;
            break;
        #endif
        #ifdef HAVE_UART4
        case UART4_BASE:
            irq = UART4_IRQn;
            pp = UART4_IT_PP;
            sp = UART4_IT_SP;
            break;
        #endif
        #ifdef HAVE_UART7
        case UART7_BASE:
            irq = UART7_IRQn;
            pp = UART7_IT_PP;
            sp = UART7_IT_SP;

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
        txDma.bindDoneCb([this](){this->invokeTxDma();});
    }
}

void UartHw::enableRxDma(const bool en){
    USART_DMACmd(instance, USART_DMAReq_Rx, en);
    if(en){
        rxDma.init(DmaChannel::Mode::toMemCircular);
        rxDma.enableIt({1,1});
        rxDma.enableDoneIt();
        rxDma.enableHalfIt();
        rxDma.bindDoneCb([this](){
            this->invokeRxDma();
            for(size_t i = rx_dma_buf_index; i < rx_dma_buf_size; i++) this->rxBuf.addData(rx_dma_buf[i]); 
            rx_dma_buf_index = 0;
        });

        rxDma.bindHalfCb([this](){
            for(size_t i = rx_dma_buf_index; i < rx_dma_buf_size / 2; i++) this->rxBuf.addData(rx_dma_buf[i]); 
            rx_dma_buf_index = rx_dma_buf_size / 2;
        });

        this->bindIdleCb([this](){
            size_t index = rx_dma_buf_size - rxDma.pending();
            if(index != rx_dma_buf_size / 2 && index != rx_dma_buf_size){
                for(size_t i = rx_dma_buf_index; i < index; i++) this->rxBuf.addData(rx_dma_buf[i]); 
            }
            rx_dma_buf_index = index;
        });

        rxDma.begin((void *)rx_dma_buf, (void *)(&instance->DATAR), rx_dma_buf_size);
    }else{
        rxDma.bindDoneCb(nullptr);
        rxDma.bindHalfCb(nullptr);
        this->bindIdleCb(nullptr);

    }
}

void UartHw::invokeTxDma(){
    if(txDma.pending() == 0 && txBuf.available() != 0){
        size_t tx_amount = 0;
        while(txBuf.available()){
            tx_dma_buf[tx_amount++] = txBuf.getData();
            if(tx_amount >= tx_dma_buf_size){
                break;
            }
        }
        txDma.begin((void *)(&instance->DATAR), (void *)tx_dma_buf, tx_amount);
    }
}

void UartHw::invokeRxDma(){
    rxDma.begin();
}


void UartHw::enableRxneIt(const bool en){
    USART_ClearITPendingBit(instance, USART_IT_RXNE);
    USART_ITConfig(instance, USART_IT_RXNE, en);
    if(en){
        this->bindRxneCb([this](){
            this->rxBuf.addData(USART_ReceiveData(instance));
        });
    }
}

void UartHw::enableIdleIt(const bool en){
    USART_ClearITPendingBit(instance, USART_IT_IDLE);
    USART_ITConfig(instance, USART_IT_IDLE, en);
}

void UartHw::enableTxeIt(const bool en){
    if(en){
        this->bindTxeCb([this](){
            if(this->txBuf.available()) instance->DATAR = this->txBuf.getData();
            else USART_ITConfig(instance, USART_IT_TXE, false);
        });
    }else{
        this->bindTxeCb(nullptr);
    }
}
void UartHw::setTxMethod(const CommMethod _txMethod){
    if(txMethod != _txMethod){
        txMethod = _txMethod;
        if(txMethod != CommMethod::None){
            Gpio & tx_pin = getTxPin();
            tx_pin.afpp();
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
            rx_pin.inpu();
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



void UartHw::init(const uint32_t baudRate, const CommMethod _txMethod, const CommMethod _rxMethod){
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

void UartHw::write(const char data){
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


