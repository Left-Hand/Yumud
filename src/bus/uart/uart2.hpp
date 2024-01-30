#ifndef __UART2_HW_HPP__

#define __UART2_HW_HPP__

#include "../printer.hpp"
#include "../../defines/comm_inc.h"

class Uart2:public Printer{
protected:
    void _write(const char & data) override;
    void _write(const char * data_ptr, const size_t & len) override;
    void _read(char & data) override;
    void _read(char * data, const size_t len) override;
    void _fake_read(const size_t len) override;
    char * _get_read_ptr() override;
public:
    void init(const uint32_t & baudRate);
    size_t available() override;
};

__interrupt 
void USART2_IRQHandler();

#endif