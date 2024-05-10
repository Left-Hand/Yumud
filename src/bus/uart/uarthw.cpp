#include "uarthw.hpp"

void UartHw::enableRcc(const bool & en){
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
            break;
        #endif
        #ifdef HAVE_UART5
        case UART5_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, en);
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
            gpio_instance = &portB;
            gpio_pin = UART1_RX_Pin;
            break;
        #endif
        #ifdef HAVE_UART2
        case USART2_BASE:
            gpio_instance = &portA;
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
            gpio_instance = UART4_RX_Port;
            gpio_pin = UART4_RX_Pin;
            break;
        #endif
        #ifdef HAVE_UART5
        case UART5_BASE:
            gpio_instance = UART5_RX_Port;
            gpio_pin = UART5_RX_Pin;
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
                gpio_instance = &portA;
                gpio_pin = UART2_TX_Pin;
                break;
        #endif
        #ifdef HAVE_UART3
        case USART3_BASE:
                gpio_instance = UART2_TX_Port;
                gpio_pin = UART2_TX_Pin;
                break;
        #endif
        default:
            break;
    }

    return (*gpio_instance)[(Pin)gpio_pin];
}

void UartHw::setupNvic(const bool & en){
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
        default:
            return;
    }

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = irqch;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = pp;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = sp;
    NVIC_InitStructure.NVIC_IRQChannelCmd = en;
    NVIC_Init(&NVIC_InitStructure);
}
void UartHw::enableRxIt(const bool & en){
    USART_ClearITPendingBit(instance, USART_IT_RXNE);
    USART_ITConfig(instance, USART_IT_RXNE, en);
}

void UartHw::enableTxIt(const bool &en){
    USART_ClearITPendingBit(instance, USART_IT_TXE);
}

void UartHw::setTxMethod(const CommMethod &_txMethod){
    if(txMethod != _txMethod){
        txMethod = _txMethod;
        switch(txMethod){
            case CommMethod::Blocking:
                break;
            case CommMethod::Interrupt:
                enableTxIt(true);
                break;
            case CommMethod::DmaNormal:
                break;
            default:
                break;
        }
    }
}

void UartHw::setRxMethod(const CommMethod &_rxMethod){
    if(rxMethod != _rxMethod){
        rxMethod = _rxMethod;
        switch(rxMethod){
            case CommMethod::Blocking:
                break;
            case CommMethod::Interrupt:
                enableRxIt(true);
                break;
            case CommMethod::DmaNormal:
                break;
            default:
                break;
        }
    }
}

void UartHw::setupDma(char * reg_ptr, char * buf_ptr, 
        size_t buf_size, DMA_Channel_TypeDef * dma_instance, const bool & buf_as_receiver){
    DMA_InitTypeDef DMA_InitStructure = {0};

    DMA_DeInit(dma_instance);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)reg_ptr;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)buf_ptr;
    DMA_InitStructure.DMA_DIR = buf_as_receiver ? DMA_DIR_PeripheralSRC : DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = (uint32_t)buf_size;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(dma_instance, &DMA_InitStructure);
    DMA_Cmd(dma_instance, ENABLE);
}

void UartHw::init(const uint32_t & baudRate, const Mode & _mode,const CommMethod & _rxMethod,const CommMethod & _txMethod){
    mode = _mode;

    if(((uint8_t)mode & (uint8_t)Mode::TxOnly)){
        Gpio & tx_pin = getTxPin();
        tx_pin.OutAfPP();
    }

    if(((uint8_t)mode & (uint8_t)Mode::RxOnly)){
        Gpio & rx_pin = getRxPin();
        rx_pin.InPullUP();
    }

    enableRcc();

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = baudRate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = (uint8_t)mode << 2;

    USART_Init(instance, &USART_InitStructure);
    USART_Cmd(instance, ENABLE);

    setupNvic(true);
    setRxMethod(_rxMethod);
    setTxMethod(_txMethod);
}


void UartHw::_write(const char * data_ptr, const size_t & len){
    switch(txMethod){
        case CommMethod::Blocking:
            instance->DATAR;
            for(size_t i=0;i<len;i++) _write(data_ptr[i]);
            while((instance->STATR & USART_FLAG_TC) == RESET);
            break;
        case CommMethod::Interrupt:
            for(size_t i=0;i<len;i++)txBuf.addData(data_ptr[i]);
            triggerTxIt();
            break;
        case CommMethod::DmaNormal:
            break;
        case CommMethod::DmaCircular:
            break;
        case CommMethod::DmaDual:
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
            while((instance->STATR & USART_FLAG_TXE) == RESET);
            break;
        case CommMethod::Interrupt:
            txBuf.addData(data);
            triggerTxIt();
            break;
        case CommMethod::DmaNormal:
            break;
        case CommMethod::DmaCircular:
            break;
        case CommMethod::DmaDual:
            break;
        default:
            break;
    }
}
