#include "uart.hpp"

using namespace ymd::hal;

size_t UartBase::try_read_byte(uint8_t & byte){
    return rx_fifo_.try_pop(std::span(&byte, 1));
}

size_t UartBase::try_read_bytes(std::span<uint8_t> bytes){
    return rx_fifo_.try_pop(bytes);
}
