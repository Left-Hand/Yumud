#include "spi2.hpp"

#ifdef HAVE_SPI2

Spi2 spi2;

int8_t Spi2::occupied = -1;

__interrupt
void SPI2_IRQHandler(void){
}
#endif