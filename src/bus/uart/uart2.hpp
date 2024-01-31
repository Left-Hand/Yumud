#ifndef __UART2_HW_HPP__

#define __UART2_HW_HPP__

#include "../printer.hpp"
#include "../../types/buffer/ringbuf/ringbuf_t.hpp"
#include "../../defines/comm_inc.h"

class Uart2:public Printer{
    __fast_inline void _write(const char & data) override;
    __fast_inline void _write(const char * data_ptr, const size_t & len) override;
public:
    RingBuf ringBuf;
    Uart2():Printer(ringBuf){;}

    void init(const uint32_t & baudRate);
};

__fast_inline void Uart2::_write(const char & data){
    while((USART2->STATR & USART_FLAG_TXE) == RESET);	   
    USART2->DATAR = data;
    while((USART2->STATR & USART_FLAG_TC) == RESET);
}

__fast_inline void Uart2::_write(const char * data_ptr, const size_t & len){
  	for(size_t i=0;i<len;i++){	
        _write(data_ptr[i]);
	}	 	
}

extern Uart2 uart2;

__interrupt 
void USART2_IRQHandler();

#endif