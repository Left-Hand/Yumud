#pragma once

#include "hal/gpio/port_virtual.hpp"
#include "hal/bus/bus.hpp"

#include "spi_trait.hpp"

namespace ymd{

class Spi:public FullDuplexBus, traits::SpiTrait{
public:
    #ifndef SPI_MAX_PINS
    #define SPI_MAX_PINS 4
    #endif
    PortVirtual <SPI_MAX_PINS> cs_port = PortVirtual<SPI_MAX_PINS>();

    #ifdef SPI_MAX_PINS
    #undef SPI_MAX_PINS
    #endif
protected:

    uint8_t last_index;
    Error lead(const uint8_t index) override{
        last_index = index;
        cs_port[last_index].clr();
        return ErrorType::OK;
    }

    void trail() override{
        cs_port[last_index].set();
    }
public:
    Spi(){;}
    DELETE_COPY_AND_MOVE(Spi)

    virtual void init(const uint32_t baudRate, const CommMethod tx_method = CommMethod::Blocking, const CommMethod rx_method = CommMethod::Blocking) = 0;
    void bindCsPin(GpioConcept & gpio, const uint8_t index){
        gpio.outpp(HIGH);
        cs_port.bindPin(gpio, index);
    }
};


}