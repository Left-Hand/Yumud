#pragma once

#include "spi.hpp"
#include "sys/clock/clock.h"


namespace ymd::hal{
class SpiSw: public Spi{
protected:
    volatile int8_t occupied = -1;
    hal::GpioIntf & sclk_gpio;
    hal::GpioIntf & mosi_gpio;
    hal::GpioIntf & miso_gpio;

    uint16_t delays = 100;
    uint8_t data_bits = 8;
    bool m_msb = true;

    void delayDur(){
        delayMicroseconds(delays);
    }


    BusError lead(const uint8_t index) override{
        auto ret = Spi::lead(index);
        delayDur();
        return ret;
    }

    void trail() override{
        sclk_gpio.set();
        delayDur();
        Spi::trail();
    }
protected :
public:

    SpiSw(hal::GpioIntf & _sclk_pin,hal::GpioIntf & _mosi_pin,
            hal::GpioIntf & _miso_pin):sclk_gpio(_sclk_pin),
                mosi_gpio(_mosi_pin), miso_gpio(_miso_pin){;}
    SpiSw(hal::GpioIntf & _sclk_pin,hal::GpioIntf & _mosi_pin,
            hal::GpioIntf & _miso_pin,hal::GpioIntf & _cs_pin):SpiSw(_sclk_pin, _mosi_pin, _miso_pin){
                bind_cs_pin(_cs_pin, 0);
            }

    DELETE_COPY_AND_MOVE(SpiSw);

    void init(const uint32_t baudrate, 
        const CommStrategy tx_strategy = CommStrategy::Blocking, 
        const CommStrategy rx_strategy = CommStrategy::Blocking) override;


    BusError write(const uint32_t data) {
        uint32_t dummy;
        transfer(dummy, data);
        return BusError::OK;
    }

    BusError read(uint32_t & data) {
        uint32_t ret;
        scexpr uint32_t dummy = 0;
        transfer(ret, dummy); 
        return BusError::OK;
    }

    BusError transfer(uint32_t & data_rx, const uint32_t data_tx) override ;

    void set_baudrate(const uint32_t baudrate) {
        if(baudrate == 0){
            delays = 0;
        }else{
            uint32_t b = baudrate / 1000;
            delays = 200 / b;
        }
    }

    void set_data_width(const uint8_t bits) override {
        data_bits = bits;
    }

    void set_bitorder(const Endian endian) override {
        m_msb = (endian == MSB);
    }
};



}