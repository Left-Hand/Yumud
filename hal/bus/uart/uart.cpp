#include "uart.hpp"

using namespace ymd::hal;

void Uart::read1(char & data){
    // __builtin_abort();
    data = rx_fifo_.pop();
}

void Uart::readN(char * data_ptr, const size_t len){
    // __builtin_abort();
    (void)rx_fifo_.pop(std::span(data_ptr, len));
}
