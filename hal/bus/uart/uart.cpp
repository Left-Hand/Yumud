#include "uart.hpp"

void Uart::read(char & data){data = rxBuf.getData();}
void Uart::read(char * data_ptr, const size_t len){rxBuf.getDatas(data_ptr, len);}
