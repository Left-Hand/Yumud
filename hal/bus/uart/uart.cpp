#include "uart.hpp"

using namespace ymd::hal;

size_t Uart::try_read_char(char & data){
    return rx_fifo_.try_pop(std::span(&data, 1));
}

size_t Uart::try_read_chars(char * data_ptr, const size_t len){
    return rx_fifo_.try_pop(std::span(data_ptr, len));
}
