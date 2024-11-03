#pragma once

#include "spi.hpp"

namespace yumud{
class SpiSw: public Spi{
protected:
    volatile int8_t occupied = -1;
    GpioConcept & sclk_gpio;
    GpioConcept & mosi_gpio;
    GpioConcept & miso_gpio;

    uint16_t delays = 100;
    uint8_t data_bits = 8;
    bool m_msb = true;

    void delayDur(){
        delayMicroseconds(delays);
    }


    Error lead(const uint8_t index) override{
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

    SpiSw(GpioConcept & _sclk_pin,GpioConcept & _mosi_pin,
            GpioConcept & _miso_pin):sclk_gpio(_sclk_pin),
                mosi_gpio(_mosi_pin), miso_gpio(_miso_pin){;}
    SpiSw(GpioConcept & _sclk_pin,GpioConcept & _mosi_pin,
            GpioConcept & _miso_pin,GpioConcept & _cs_pin):SpiSw(_sclk_pin, _mosi_pin, _miso_pin){
                bindCsPin(_cs_pin, 0);
            }
    Error write(const uint32_t data) override {
        uint32_t dummy;
        transfer(dummy, data, false);
        return Bus::ErrorType::OK;
    }

    Error read(uint32_t & data, bool toAck = true) {
        uint32_t ret;
        scexpr uint32_t dummy = 0;
        transfer(ret, dummy, toAck); 
        return Bus::ErrorType::OK;
    }

    Error transfer(uint32_t & data_rx, const uint32_t data_tx, bool toAck = true) override ;

    void setBaudRate(const uint32_t baudRate) {
        if(baudRate == 0){
            delays = 0;
        }else{
            uint32_t b = baudRate / 1000;
            delays = 200 / b;
        }
    }

    void init(const uint32_t baudRate, const CommMethod tx_method = CommMethod::Blocking, const CommMethod rx_method = CommMethod::Blocking) override;

    void setDataBits(const uint8_t bits) override {
        data_bits = bits;
    }

    void setBitOrder(const Endian endian) override {
        m_msb = bool(endian);
    }
};



}