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

#ifdef HAVE_UART1
UART_CB_TEMPLATE(uart1)
UART_IT_TEMPLATE(uart1, USART1)
UartHw uart1{USART1, UART1_TX_DMA_CH, UART1_RX_DMA_CH};
#endif


#ifdef HAVE_UART2
UART_CB_TEMPLATE(uart2)
UART_IT_TEMPLATE(uart2, USART2)
UartHw uart2{USART2, UART2_TX_DMA_CH, UART2_RX_DMA_CH};
#endif

#ifdef HAVE_UART3
UART_CB_TEMPLATE(uart3)
UART_IT_TEMPLATE(uart3, USART3)
UartHw uart3{USART3, UART3_TX_DMA_CH, UART3_RX_DMA_CH};
#endif


#ifdef HAVE_UART4
UART_CB_TEMPLATE(uart4)
UART_IT_TEMPLATE(uart4, UART4)
UartHw uart4{UART4, UART4_TX_DMA_CH, UART4_RX_DMA_CH};
#endif

#ifdef HAVE_UART5
UART_CB_TEMPLATE(uart5)
UART_IT_TEMPLATE(uart5, UART5)
UartHw uart5{UART5, UART5_TX_DMA_CH, UART5_RX_DMA_CH};
#endif


#ifdef HAVE_UART6
UART_CB_TEMPLATE(uart6)
UART_IT_TEMPLATE(uart6, UART6)
UartHw uart6{UART6, UART6_TX_DMA_CH, UART6_RX_DMA_CH};
#endif

#ifdef HAVE_UART7
UART_CB_TEMPLATE(uart7)
UART_IT_TEMPLATE(uart7, UART7)
UartHw uart7{UART7, UART7_TX_DMA_CH, UART7_RX_DMA_CH};
#endif


#ifdef HAVE_UART8
UART_CB_TEMPLATE(uart8)
UART_IT_TEMPLATE(uart8, UART8)
UartHw uart8{UART8, UART8_TX_DMA_CH, UART8_RX_DMA_CH};
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
        #ifdef HAVE_UART7
        case UART7_BASE:
            uart7_rxne_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART8
        case UART8_BASE:
            uart8_rxne_cb = cb;
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
        #ifdef HAVE_UART7
        case UART7_BASE:
            uart7_txe_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART8
        case UART8_BASE:
            uart8_txe_cb = cb;
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
        #ifdef HAVE_UART7
        case UART7_BASE:
            uart7_idle_cb = cb;
            break;
        #endif
        #ifdef HAVE_UART8
        case UART8_BASE:
            uart8_idle_cb = cb;
            break;
        #endif
    }
}

void UartHw::enableRcc(const bool en){
    switch((uint32_t)instance){
        #ifdef HAVE_UART1
        case USART1_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, en);
            if(UART1_REMAP){
                GPIO_PinRemapConfig(GPIO_Remap_USART1, true);
            }
            break;
        #endif
        #ifdef HAVE_UART2
        case USART2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, en);
            if(UART2_REMAP){
                GPIO_PinRemapConfig(GPIO_Remap_USART2, true);
            }
            break;
        #endif
        #ifdef HAVE_UART3
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
        #ifdef HAVE_UART4
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
        #ifdef HAVE_UART5
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
        #ifdef HAVE_UART6
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
        #ifdef HAVE_UART7
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
        #ifdef HAVE_UART8
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


Gpio & UartHw::getRxPin(){
    switch((uint32_t)instance){
        #ifdef HAVE_UART1
        case USART1_BASE:
            return UART1_RX_Gpio;
        #endif
        #ifdef HAVE_UART2
        case USART2_BASE:
            return UART2_RX_Gpio;
        #endif
        #ifdef HAVE_UART3
        case USART3_BASE:
            return UART3_RX_Gpio;
        #endif
        #ifdef HAVE_UART4
        case UART4_BASE:
            return UART4_RX_Gpio;
        #endif
        #ifdef HAVE_UART5
        case UART5_BASE:
            return UART5_RX_Gpio;
        #endif
        #ifdef HAVE_UART6
        case UART6_BASE:
            return UART6_RX_Gpio;
        #endif
        #ifdef HAVE_UART7
        case UART7_BASE:
            return UART7_RX_Gpio;
        #endif
        #ifdef HAVE_UART8
        case UART8_BASE:
            return UART8_RX_Gpio;
        #endif
        default:
            return GpioNull;
    }
}

Gpio & UartHw::getTxPin(){
    switch((uint32_t)instance){
        #ifdef HAVE_UART1
        case USART1_BASE:
            return UART1_TX_Gpio;
        #endif
        #ifdef HAVE_UART2
        case USART2_BASE:
            return UART2_TX_Gpio;
        #endif
        #ifdef HAVE_UART3
        case USART3_BASE:
            return UART3_TX_Gpio;
        #endif
        #ifdef HAVE_UART4
        case UART4_BASE:
            return UART4_TX_Gpio;
        #endif
        #ifdef HAVE_UART5
        case UART5_BASE:
            return UART5_TX_Gpio;
        #endif
        #ifdef HAVE_UART6
        case UART6_BASE:
            return UART6_TX_Gpio;
        #endif
        #ifdef HAVE_UART7
        case UART7_BASE:
            return UART7_TX_Gpio;
        #endif
        #ifdef HAVE_UART8
        case UART8_BASE:
            return UART8_TX_Gpio;
        #endif
        default:
            return GpioNull;
    }
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
        #ifdef HAVE_UART5
        case UART5_BASE:
            irq = UART5_IRQn;
            pp = UART5_IT_PP;
            sp = UART5_IT_SP;
            break;
        #endif
        #ifdef HAVE_UART6
        case UART6_BASE:
            irq = UART6_IRQn;
            pp = UART6_IT_PP;
            sp = UART6_IT_SP;
            break;
        #endif
        #ifdef HAVE_UART7
        case UART7_BASE:
            irq = UART7_IRQn;
            pp = UART7_IT_PP;
            sp = UART7_IT_SP;
            break;
        #endif
        #ifdef HAVE_UART8
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
            EXECUTE(rxPostCb);
        });

        rxDma.begin((void *)rx_dma_buf, (void *)(&instance->DATAR), rx_dma_buf_size);
    }else{
        rxDma.bindDoneCb(nullptr);
        rxDma.bindHalfCb(nullptr);
        this->bindIdleCb(nullptr);

    }
}

void UartHw::invokeTxDma(){
    if(txDma.pending() == 0){
        if(txBuf.available()){
            size_t tx_amount = 0;
            while(txBuf.available()){
                tx_dma_buf[tx_amount++] = txBuf.getData();
                if(tx_amount >= tx_dma_buf_size){
                    break;
                }
            }
            txDma.begin((void *)(&instance->DATAR), (void *)tx_dma_buf, tx_amount);
        }else{
            EXECUTE(txPostCb);
        }
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


