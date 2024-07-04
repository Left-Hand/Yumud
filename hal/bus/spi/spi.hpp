#pragma once

#include "hal/gpio/gpio.hpp"
#include "hal/gpio/port_virtual.hpp"
#include "hal/bus/bus.hpp"

class Spi:public FullDuplexBus{
protected:
    static constexpr uint8_t spi_max_cs_pins = 4;
    PortVirtual <spi_max_cs_pins> cs_port = PortVirtual<spi_max_cs_pins>();

    Error lead(const uint8_t index) override{
        cs_port[wholock()].clr();
        return ErrorType::OK;
    }

    void trail() override{
        cs_port[wholock()].set();
    }
public:

    void bindCsPin(GpioConcept & gpio, const uint8_t index){
        cs_port.bindPin(gpio, index);
    }
};


