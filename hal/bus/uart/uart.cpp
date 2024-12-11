#include "uart.hpp"

using namespace ymd;

void Uart::read(char & data){data = rxBuf.pop();}
void Uart::read(char * data_ptr, const size_t len){rxBuf.pop(data_ptr, len);}

void Uart::flush(){
    //TODO
};
