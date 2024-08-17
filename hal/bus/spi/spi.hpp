#pragma once

#include "../hal/gpio/gpio.hpp"
#include "../hal/gpio/port_virtual.hpp"
#include "../hal/bus/bus.hpp"

class Spi:public FullDuplexBus{
public:
    #ifndef SPI_MAX_PINS
    #define SPI_MAX_PINS 4
    #endif
    PortVirtual <SPI_MAX_PINS> cs_port = PortVirtual<SPI_MAX_PINS>();

    #ifdef SPI_MAX_PINS
    #undef SPI_MAX_PINS
    #endif
protected:

    Error lead(const uint8_t index) override{
        cs_port[wholock()].clr();
        return ErrorType::OK;
    }

    void trail() override{
        cs_port[wholock()].set();
    }
public:
    virtual void init(const uint32_t baudRate, const CommMethod tx_method = CommMethod::Blocking, const CommMethod rx_method = CommMethod::Blocking) = 0;
    void bindCsPin(GpioConcept & gpio, const uint8_t index){
        gpio.outpp(1);
        cs_port.bindPin(gpio, index);
    }
};


