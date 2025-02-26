#pragma once

#include "hal/gpio/port_virtual.hpp"
#include "hal/bus/bus.hpp"


namespace ymd::hal{

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
    CommMethod tx_method_;
    CommMethod rx_method_;
    uint8_t last_index;

    BusError lead(const uint8_t index) override{
        last_index = index;
        cs_port[last_index].clr();
        return BusError::OK;
    }

    void trail() override{
        cs_port[last_index].set();
    }
public:
    Spi(){;}
    Spi(const Spi &) = delete;
    Spi(Spi &&) = delete;

    virtual void setDataBits(const uint8_t len) = 0;
    virtual void setBaudRate(const uint32_t baud) = 0;
    virtual void setBitOrder(const Endian endian) = 0;

    virtual void init(
        const uint32_t baudRate, 
        const CommMethod tx_method = CommMethod::Blocking, 
        const CommMethod rx_method = CommMethod::Blocking) = 0;
    void bindCsPin(GpioConcept & gpio, const uint8_t index){
        gpio.outpp(HIGH);
        cs_port.bindPin(gpio, index);
    }
};


}