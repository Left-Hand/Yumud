#pragma once

#include "spi.hpp"

class SpiSw: public Spi{
protected:
    volatile int8_t occupied = -1;
    GpioConcept & sclk_pin;
    GpioConcept & mosi_pin;
    GpioConcept & miso_pin;

    uint16_t delays = 100;
    uint8_t data_size = 8;

    __fast_inline void delayDur(){
        // volatile uint16_t i = delays;
        // while(i--) __nop;
    }

    void clk(){
        // delayDur();
        sclk_pin.set();
        // delayDur();
        sclk_pin.clr();
    }

    void clkr(){
        // delayDur();
        sclk_pin.clr();
        // delayDur();
        sclk_pin.set();
        // delayDur();
        sclk_pin.clr();
        // delayDur();
    }

    void clk_up(){
        // delayDur();
        sclk_pin.clr();
        // delayDur();
        sclk_pin.set();
        // delayDur();
    }

    void clk_down(){
        sclk_pin.clr();
    }

    void clk_down_then_up(){
        clk_down();
        clk_up();
    }

protected :
public:

    SpiSw(GpioConcept & _sclk_pin,GpioConcept & _mosi_pin,
            GpioConcept & _miso_pin, const uint16_t & _delays = 10):sclk_pin(_sclk_pin),
                mosi_pin(_mosi_pin), miso_pin(_miso_pin), delays(_delays){;}
    SpiSw(GpioConcept & _sclk_pin,GpioConcept & _mosi_pin,
            GpioConcept & _miso_pin,GpioVirtual & _cs_pin, const uint16_t & _delays = 10):SpiSw(_sclk_pin, _mosi_pin, _miso_pin, delays){
                bindCsPin(_cs_pin, 0);
            }
    Error write(const uint32_t & data) override {
        delayDur();

        for(uint16_t mask = 1 << (data_size - 1); mask; mask >>= 1){
            mosi_pin.write(mask & data);
            clk();
        }

        return Bus::ErrorType::OK;
    }

    Error read(uint32_t & data, bool toAck = true) {
        uint8_t ret = 0;
        delayDur();

        clk_up();
        ret |= miso_pin.read();
        for(uint8_t i = 0; i < data_size - 1; i++){
            clk_down_then_up();
            ret <<= 1; ret |= miso_pin.read();
        }

        clk_down();

        data = ret;
        return Bus::ErrorType::OK;
    }

    Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck){
        uint8_t ret = 0;
        delayDur();

        mosi_pin = data_tx & (1 << (data_size - 1));
        clk_up();
        ret |= miso_pin.read();
        for(uint8_t i = 0; i < data_size - 1; i++){
            clk_down();
            mosi_pin = bool(data_tx & (1 << (data_size - 2 - i)));
            clk_up();
            ret <<= 1; ret |= miso_pin.read();
        }

        clk_down();

        data_rx = ret;
        return Bus::ErrorType::OK;
    }

    void init(const uint32_t & baudRate){
        configBaudRate(baudRate);
        init();
    }

    void init(){
        preinit();
        mosi_pin.outpp();
        sclk_pin.outpp();

        for(uint8_t i = 0; i < cs_pins.length(); i++){
            cs_pins.setModeByIndex(i, PinMode::OutPP);
        }

        miso_pin.inflt();
    }
};

