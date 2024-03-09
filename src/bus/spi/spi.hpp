#ifndef __SPI_HPP__

#define __SPI_HPP__

#include "src/gpio/port.hpp"
#include "src/bus/serbus.hpp"

class Spi:public SerBus{
public:
};

class SpiHw:public Spi{
protected:
    SPI_TypeDef * instance;

    Gpio getMosiPin();
    Gpio getMisoPin();
    Gpio getSclkPin();
    Gpio getCsPin();

    void enableRcc(const bool en = true);
    uint16_t calculatePrescaler(const uint32_t baudRate);
    void initGpios();
public:
    SpiHw(SPI_TypeDef * _instance):instance(_instance){;}
    void init(const uint32_t baudRate);
    void enableHwCs(const bool en = true);
    void enableRxIt(const bool en = true);
};

#endif