#ifndef __SPI1_HPP__

#define __SPI1_HPP__

#include "spi.hpp"


class Spi1:public SpiHw{
protected:
    int8_t occupied;

    void lock(const uint8_t &index) override{occupied = index;}
    void unlock() override{occupied = -1;}
    int8_t wholock() override{return occupied;}
public:

    Spi1():SpiHw(SPI1){;}
};

#ifdef HAVE_SPI1
extern Spi1 spi1;

extern"C"{
__interrupt
void SPI1_IRQHandler(void);
}
#endif
#endif