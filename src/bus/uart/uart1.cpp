#include "uart1.hpp"

void Uart1::init(const uint32_t & baudRate){
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    USART_InitStructure.USART_BaudRate = baudRate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

void Uart1::_write(const char & data){
    __nopn(8);
    USART_SendData(USART1, data);
        __nopn(2); 
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
    __nopn(8);
}

void Uart1::_write(const char * data_ptr, const size_t & len){
    __nopn(8);
  	for(size_t i=0;i<len;i++){		   
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
        __nopn(2); 
		USART_SendData(USART1,data_ptr[i]);
	}	 
 
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);		
    __nopn(8);
}

void Uart1::_read(char & data){
    ringBuf.getRxData((uint8_t &)data);
}

void Uart1::_read(char * data, const size_t len){
    ringBuf.getRxDatas((uint8_t *)data, len);
}