#include "uarthw.hpp"
#include "uarts.hpp"


static constexpr size_t rx_dma_buf_size = 32;

#ifdef HAVE_UART1
UartHw uart1{USART1, UART1_TX_DMA_CH, UART1_RX_DMA_CH};

static char u1rx_dma_buf[rx_dma_buf_size];

__interrupt
void USART1_IRQHandler(void){

    if(USART_GetITStatus(USART1,USART_IT_RXNE)){
        USART_ClearITPendingBit(USART1,USART_IT_RXNE);
        uart1.rxBuf.addData(USART_ReceiveData(USART1));
    }else if(USART_GetITStatus(USART1,USART_IT_IDLE)){
        USART_ClearITPendingBit(USART1,USART_IT_IDLE);
    }else if(USART_GetITStatus(USART1,USART_IT_TXE)){
        USART_ClearITPendingBit(USART1,USART_IT_TXE);
        if(uart1.txBuf.available()){
            USART1->DATAR = uart1.txBuf.getData();
        }else{
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        }
    }else if(USART_GetFlagStatus(USART1,USART_FLAG_ORE)){
        USART_ClearFlag(USART1,USART_FLAG_ORE);
        USART_ReceiveData(USART1);
    }
}

//uart1 rx
__interrupt void DMA1_Channel5_IRQHandler(void){
    if(DMA_GetFlagStatus(DMA1_IT_TC5)){
        DMA_ClearFlag(DMA1_IT_TC5);
        // USART1->DATAR = 'c';
    }else if(DMA_GetFlagStatus(DMA1_IT_HT5)){
        DMA_ClearFlag(DMA1_IT_HT5);
        // USART1->DATAR = 't';
    }
}


//uart1 tx
__interrupt void DMA1_Channel4_IRQHandler(void){
    if(DMA_GetFlagStatus(DMA1_IT_TC4)){
        static size_t last_amount = 0;
        uart1.txBuf.readForward(last_amount);

        if(uart1.txBuf.available()){
            uart1.invokeTxDma(last_amount = uart1.txBuf.straight());
        }

        DMA_ClearFlag(DMA1_IT_TC4);
    }else if(DMA_GetFlagStatus(DMA1_IT_HT4)){
        DMA_ClearFlag(DMA1_IT_HT4);
    }
}
#endif

#ifdef HAVE_UART2
UartHw uart2{USART2, UART2_TX_DMA_CH, UART2_RX_DMA_CH};

__interrupt
void USART2_IRQHandler(void){
    if(USART_GetITStatus(USART2,USART_IT_RXNE)){
        USART_ClearITPendingBit(USART2,USART_IT_RXNE);
        uart2.rxBuf.addData(USART_ReceiveData(USART2));
    }else if(USART_GetITStatus(USART2,USART_IT_IDLE)){

        DMA_Cmd(DMA1_Channel6, DISABLE);
        USART2->STATR;
        USART2->DATAR;//clear idle flag
    }else if(USART_GetITStatus(USART2,USART_IT_TXE)){
        USART_ClearITPendingBit(USART2,USART_IT_TXE);
        if(uart2.txBuf.available()){
            USART2->DATAR = uart2.txBuf.getData();
        }else{
            USART_ITConfig(USART2, USART_IT_TXE, false);
        }
    }else if(USART_GetFlagStatus(USART2,USART_FLAG_ORE)){
        USART_ClearFlag(USART2,USART_FLAG_ORE);
        USART_ReceiveData(USART2);
    }
}

//uart2 tx
__interrupt void DMA1_Channel7_IRQHandler(void){
    if(DMA_GetFlagStatus(DMA1_IT_TC7)){
        DMA_ClearFlag(DMA1_IT_TC7);

        static size_t last_amount = 0;
        uart2.txBuf.readForward(last_amount);

        if(uart2.txBuf.available()){
            uart2.invokeTxDma(last_amount = uart2.txBuf.straight());
        }
    }else if(DMA_GetFlagStatus(DMA1_IT_HT7)){
        DMA_ClearFlag(DMA1_IT_HT7);
    }
}


//uart2 rx
__interrupt void DMA1_Channel6_IRQHandler(void){
    if(DMA_GetFlagStatus(DMA1_IT_TC6)){
        DMA_ClearFlag(DMA1_IT_TC6);
        USART2->DATAR = 'c';
    }else if(DMA_GetFlagStatus(DMA1_IT_HT6)){
        DMA_ClearFlag(DMA1_IT_HT6);
        USART2->DATAR = 't';
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

    NvicRequest(pp, sp, IRQn(irqch)).enable();
}

void UartHw::invokeTxIt(){
    USART_ITConfig(instance, USART_IT_TXE, ENABLE);
}


void UartHw::enableRxDma(const bool en){
    if(en){
        rxDma.init(DmaChannel::Mode::toMemCircular);
        rxDma.enableDoneIt();
        rxDma.enableHalfIt();
        rxDma.enableIt({0,0});
        rxDma.begin((void *)u1rx_dma_buf, (void *)(&instance->DATAR), rx_dma_buf_size);
    }
    USART_DMACmd(instance, USART_DMAReq_Rx, en);
}


void UartHw::enableTxDma(const bool en){
    txDma.init(DmaChannel::Mode::toPeriph);
    txDma.enableIt({1,1});
    txDma.enableDoneIt();
    USART_DMACmd(instance, USART_DMAReq_Tx, en);
}

void UartHw::invokeTxDma(const size_t amount){
    if(txDma.pending() == 0 && txBuf.available() != 0){
        txDma.begin((void *)(&instance->DATAR), (void *)txBuf.read_ptr, amount);
        // txDma.begin((void *)(&instance->DATAR), (void *)txBuf.buf, (txBuf.write_ptr- txBuf.buf));
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
        txMethod = _txMethod;
        if(txMethod != CommMethod::None){
            Gpio & tx_pin = getTxPin();
            tx_pin.OutAfPP();
        }
        switch(txMethod){
            case CommMethod::Blocking:
                break;
            case CommMethod::Interrupt:
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
    setRxMethod(_rxMethod);
    setTxMethod(_txMethod);
    USART_Cmd(instance, ENABLE);
}


void UartHw::_write(const char * data_ptr, const size_t & len){
    switch(txMethod){
        case CommMethod::Blocking:
            instance->DATAR;
            for(size_t i=0;i<len;i++){
                instance->DATAR = (uint8_t)data_ptr[i];
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
            invokeTxDma(txBuf.straight());
            break;
        default:
            break;
    }
}

void UartHw::_write(const char & data){
    switch(txMethod){
        case CommMethod::Blocking:
            instance->DATAR;
            instance->DATAR = data;
            while((instance->STATR & USART_FLAG_TC) == RESET);
            break;
        case CommMethod::Interrupt:
            txBuf.addData(data);
            invokeTxIt();
            break;
        case CommMethod::Dma:
            txBuf.addData(data);
            invokeTxDma(txBuf.straight());
            break;
        default:
            break;
    }
}


