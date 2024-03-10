#ifndef __SPI2_HPP__

#define __SPI2_HPP__

#include "spi.hpp"


class Spi2:public SpiHw{
protected:
    int8_t occupied;

    void lock(const uint8_t &index) override{occupied = index;}
    void unlock() override{occupied = -1;}
    int8_t wholock() override{return occupied;}
public:

    Spi2():SpiHw(SPI2){;}
};

#ifdef HAVE_SPI2
extern Spi2 spi2;

extern"C"{
__interrupt
void SPI2_IRQHandler(void);
}
#endif
#endif