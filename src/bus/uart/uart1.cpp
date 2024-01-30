#include "uart1.hpp"

static RingBuf ringBuf;

static void NVIC_Configuration(void){
    CHECK_INIT
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void UART1_GPIO_Configuration(void){
    CHECK_INIT
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef  GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void UART1_Configuration(uint32_t baudRate){
    CHECK_INIT
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate = baudRate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

static void UART1_IT_Configuration(){
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void Uart1::init(const uint32_t & baudRate){
    UART1_GPIO_Configuration();
    UART1_Configuration(baudRate);
    UART1_IT_Configuration();
    NVIC_Configuration();
}

void Uart1::_write(const char & data){
    USART_SendData(USART1, data);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
}

void Uart1::_write(const char * data_ptr, const size_t & len){
  	for(size_t i=0;i<len;i++){		   
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
		USART_SendData(USART1,data_ptr[i]);
	}	 
 
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);		
}

void Uart1::_read(char & data){
    ringBuf.getRxData((uint8_t &)data);
}

void Uart1::_read(char * data, const size_t len){
    ringBuf.getRxDatas((uint8_t *)data, len);
}

void Uart1::_fake_read(const size_t len){
    ringBuf.waste(len);
}

char * Uart1::_get_read_ptr(){
    return (char *)(ringBuf.rxPtr());
}

size_t Uart1::available(){
    return ringBuf.available();
}

__interrupt 
void USART1_IRQHandler() 
{ 
    if(USART_GetITStatus(USART1,USART_IT_RXNE) != RESET)
    { 
        USART_ClearITPendingBit(USART1,USART_IT_RXNE);
        ringBuf.addRxData(USART_ReceiveData(USART1));
    } 
    if(USART_GetFlagStatus(USART1,USART_FLAG_ORE) == SET)
    { 
        USART_ClearFlag(USART1,USART_FLAG_ORE);
        USART_ReceiveData(USART1);
    } 
}