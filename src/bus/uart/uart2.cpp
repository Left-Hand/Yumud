#include "uart2.hpp"

static RingBuf ringBuf;

static void NVIC_Configuration(void){
    CHECK_INIT
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void UART2_GPIO_Configuration(void){
    CHECK_INIT
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef  GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = UART2_TX_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(UART2_Port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = UART2_RX_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(UART2_Port, &GPIO_InitStructure);
}

static void UART2_Configuration(uint32_t baudRate){
    CHECK_INIT
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate = baudRate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);
}

static void UART2_IT_Configuration(){
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

void Uart2::init(const uint32_t & baudRate){
    UART2_GPIO_Configuration();
    UART2_Configuration(baudRate);
    UART2_IT_Configuration();
    NVIC_Configuration();
}

#ifndef HAVE_UART2
Uart2 uart2;
#define HAVE_UART2
#endif

__interrupt
void USART2_IRQHandler(){
    if(USART_GetITStatus(USART2,USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(USART2,USART_IT_RXNE);
        ringBuf.addData(USART_ReceiveData(USART2));
    }
    if(USART_GetFlagStatus(USART2,USART_FLAG_ORE) == SET)
    {
        USART_ClearFlag(USART2,USART_FLAG_ORE);
        USART_ReceiveData(USART2);
    }
}