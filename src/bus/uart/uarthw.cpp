#include "uart.hpp"

void Uart::_read(char & data){ringBuf.getData((uint8_t &)data);}
void Uart::_read(char * data_ptr, const size_t len){ringBuf.getDatas((uint8_t *)data_ptr, len);}


void UartHw::enableRcc(const bool en){
    switch((uint32_t)instance){
        #ifdef HAVE_UART1
        case USART1_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, en);
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


Gpio UartHw::getRxPin(){
    using Pin = Gpio::Pin;

    GPIO_TypeDef * gpio_instance = GPIOA;
    uint16_t gpio_pin = 0;

    switch((uint32_t)instance){
        #ifdef HAVE_UART1
        case USART1_BASE:
            gpio_instance = UART1_RX_Port;
            gpio_pin = UART1_RX_Pin;
            break;
        #endif
        #ifdef HAVE_UART2
        case USART2_BASE:
            gpio_instance = UART2_RX_Port;
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
            break;
    }

    return Gpio(gpio_instance, (Pin)gpio_pin);
}

Gpio UartHw::getTxPin(){
    using Pin = Gpio::Pin;

    GPIO_TypeDef * gpio_instance = GPIOA;
    uint16_t gpio_pin = 0;

    switch((uint32_t)instance){
        #ifdef HAVE_UART1
        case USART1_BASE:
                gpio_instance = UART1_TX_Port;
                gpio_pin = UART1_TX_Pin;
                break;
        #endif
        #ifdef HAVE_UART2
        case USART2_BASE:
                gpio_instance = UART2_TX_Port;
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

    return Gpio(gpio_instance, (Pin)gpio_pin);
}

void UartHw::enableRxIt(const bool en){

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

    USART_ITConfig(instance, USART_IT_RXNE, en);
}

void UartHw::init(const uint32_t & baudRate, const Mode _mode){
    mode = _mode;

    if(((uint8_t)mode & (uint8_t)Mode::TxOnly)){
        Gpio tx_pin = getTxPin();
        tx_pin.OutAfPP();
    }

    if(((uint8_t)mode & (uint8_t)Mode::RxOnly)){
        Gpio rx_pin = getRxPin();
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

    enableRxIt();
}

void UartHw::_write(const char * data_ptr, const size_t & len){
    for(size_t i=0;i<len;i++) _write(data_ptr[i]);
    while((instance->STATR & USART_FLAG_TC) == RESET);
}

void UartHw::_write(const char & data){
    instance->DATAR = data;
    while((instance->STATR & USART_FLAG_TXE) == RESET);
}
