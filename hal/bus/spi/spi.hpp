#pragma once

#include "hal/gpio/vport.hpp"
#include "hal/bus/bus_base.hpp"

namespace ymd::hal{


class SpiSlaveIndex{
    public:
        explicit constexpr SpiSlaveIndex(const uint16_t spi_idx):
            spi_idx_(spi_idx){}
    
        uint8_t as_u8() const {return spi_idx_;}
    private:
        uint8_t spi_idx_;
    };


class Spi:public BusBase{
public:

    #ifndef SPI_MAX_PINS
    static constexpr size_t SPI_MAX_PINS = 4;
    #endif


    VGpioPort <SPI_MAX_PINS> cs_port = VGpioPort<SPI_MAX_PINS>();
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

    
    virtual BusError read(uint32_t & data) = 0;
    virtual BusError write(const uint32_t data) = 0;
    virtual BusError transfer(uint32_t & data_rx, const uint32_t data_tx) = 0;

    virtual void set_data_width(const uint8_t len) = 0;
    virtual void set_baudrate(const uint32_t baud) = 0;
    virtual void set_bitorder(const Endian endian) = 0;

    virtual void init(
        const uint32_t baudrate, 
        const CommStrategy tx_strategy = CommStrategy::Blocking, 
        const CommStrategy rx_strategy = CommStrategy::Blocking) = 0;
    void bind_cs_pin(hal::GpioIntf & gpio, const uint8_t index){
        gpio.outpp(HIGH);
        cs_port.bind_pin(gpio, index);
    }
};

}