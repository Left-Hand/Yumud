#ifndef __UART1_HW_HPP__

#define __UART1_HW_HPP__

#include "../printer.hpp"
#include "../../types/buffer/ringbuf/ringbuf_t.hpp"
#include "../../defines/comm_inc.h"

class Uart1:public Printer{
    __fast_inline void _write(const char & data) override;
    __fast_inline void _write(const char * data_ptr, const size_t & len) override;
public:
    RingBuf ringBuf;
    Uart1():Printer(ringBuf){;}

    void init(const uint32_t & baudRate);
    void setBaudRate(const uint32_t & baudRate);
};

__fast_inline void Uart1::_write(const char & data){
    while((USART1->STATR & USART_FLAG_TXE) == RESET);
    USART1->DATAR = data;
    while((USART1->STATR & USART_FLAG_TC) == RESET);
}

__fast_inline void Uart1::_write(const char * data_ptr, const size_t & len){
    for(size_t i=0;i<len;i++){
        _write(data_ptr[i]);
	}
}

extern Uart1 uart1;

__interrupt
void USART1_IRQHandler();

#endif