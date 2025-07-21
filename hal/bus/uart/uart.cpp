#include "uart.hpp"

using namespace ymd::hal;

void Uart::read1(char & data){data = rx_fifo_.pop();}
void Uart::readN(char * data_ptr, const size_t len){
    (void)rx_fifo_.pop(std::span(data_ptr, len));}
