#include "uart.hpp"

using namespace ymd::hal;

void Uart::read1(char & data){data = rx_fifo.pop();}
void Uart::readN(char * data_ptr, const size_t len){rx_fifo.pop(data_ptr, len);}
