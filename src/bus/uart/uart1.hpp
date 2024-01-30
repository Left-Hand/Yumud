#ifndef __UART1_HW_HPP__

#define __UART1_HW_HPP__

#include "../printer.hpp"
#include "../../defines/comm_inc.h"

class Uart1:public Printer{
private:
    RingBuf ringBuf;
protected:
    void _write(const char & data) override;
    void _write(const char * data_ptr, const size_t & len) override;
    void _read(char & data) override;
    void _read(char * data, const size_t len) override;
    char * _get_read_ptr() override {return (char *)(ringBuf.rxPtr());}
public:
    void init(const uint32_t & baudRate);
    size_t available() override {return ringBuf.available();}
};

#endif