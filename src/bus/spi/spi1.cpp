#include "spi1.hpp"

#ifdef HAVE_SPI1

Spi1 spi1;

int8_t Spi1::occupied = -1;

__interrupt
void SPI1_IRQHandler(void){
}
#endif