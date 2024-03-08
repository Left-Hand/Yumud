#include "uart.hpp"

static void UART_RCC_ON(USART_TypeDef * instance){
    if (instance == USART1) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    } else if (instance == USART2) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    } else if (instance == USART3) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    }
}


Gpio UartHw::getRxPin(USART_TypeDef * _instance, const Uart::Mode mode){
    using Pin = Gpio::Pin;

    GPIO_TypeDef * gpio_instance = GPIOA;
    uint16_t gpio_pin = 0;

    if(!((uint8_t)mode & (uint8_t)Uart::Mode::RxOnly)) return Gpio(gpio_instance, (Pin)gpio_pin);

    switch((uint32_t)_instance){
        default:
        case USART1_BASE:
            gpio_instance = UART1_RX_Port;
            gpio_pin = UART1_RX_Pin;
            break;
        case USART2_BASE:
            gpio_instance = UART2_RX_Port;
            gpio_pin = UART2_RX_Pin;
            break;
    }

    return Gpio(gpio_instance, (Pin)gpio_pin);
}

Gpio UartHw::getTxPin(USART_TypeDef * _instance, const Uart::Mode mode){
    using Pin = Gpio::Pin;

    GPIO_TypeDef * gpio_instance = GPIOA;
    uint16_t gpio_pin = 0;

    if(!((uint8_t)mode & (uint8_t)Uart::Mode::TxOnly)) return Gpio(gpio_instance, (Pin)gpio_pin);

    switch((uint32_t)_instance){
        default:
        case USART1_BASE:
            gpio_instance = UART1_TX_Port;
            gpio_pin = UART1_TX_Pin;
            break;
        case USART2_BASE:
            gpio_instance = UART2_TX_Port;
            gpio_pin = UART2_TX_Pin;
            break;
    }

    return Gpio(gpio_instance, (Pin)gpio_pin);
}

void UartHw::initRxIt(){

    uint8_t irqch;
    uint8_t pp;
    uint8_t sp;

    switch((uint32_t)instance){
        default:
        case USART1_BASE:
            irqch = USART1_IRQn;
            pp = UART1_IT_PP;
            sp = UART1_IT_SP;
            break;
        case USART2_BASE:
            irqch = USART2_IRQn;
            pp = UART2_IT_PP;
            sp = UART2_IT_SP;
            break;
    }

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = irqch;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = pp;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = sp;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(instance, USART_IT_RXNE, ENABLE);
}

void UartHw::init(const uint32_t & baudRate){
    tx_pin.OutAfPP();
    rx_pin.InPullUP();

    UART_RCC_ON(instance);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = baudRate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = (uint8_t)mode << 2;

    USART_Init(instance, &USART_InitStructure);
    USART_Cmd(instance, ENABLE);

    initRxIt();
}