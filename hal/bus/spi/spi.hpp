#pragma once

#include "hal/gpio/port_virtual.hpp"
#include "hal/bus/bus.hpp"


namespace ymd::hal{

class Spi:public FullDuplexBus{
public:
    #ifndef SPI_MAX_PINS
    #define SPI_MAX_PINS 4
    #endif
    VGpioPort <SPI_MAX_PINS> cs_port = VGpioPort<SPI_MAX_PINS>();

    #ifdef SPI_MAX_PINS
    #undef SPI_MAX_PINS
    #endif
protected:
    CommStrategy tx_strategy_;
    CommStrategy rx_strategy_;
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
    Spi(const hal::Spi &) = delete;
    Spi(hal::Spi &&) = delete;

    virtual void set_data_width(const uint8_t len) = 0;
    virtual void set_baudrate(const uint32_t baud) = 0;
    virtual void set_bitorder(const Endian endian) = 0;

    virtual void init(
        const uint32_t baudrate, 
        const CommStrategy tx_strategy = CommStrategy::Blocking, 
        const CommStrategy rx_strategy = CommStrategy::Blocking) = 0;
    void bind_cs_pin(hal::GpioIntf & gpio, const uint8_t index){
        gpio.outpp(HIGH);
        cs_port.bindPin(gpio, index);
    }
};


}