#pragma once

#include "I2s.hpp"
#include "hal/gpio/gpio.hpp"

namespace ymd::hal{

class I2sSw: public I2s{
private:
volatile int8_t occupied = -1;
hal::GpioIntf & sck;
hal::GpioIntf & sda;
hal::GpioIntf & ws;

uint16_t delays = 0;

__fast_inline volatile void delayDur(){
    if(delays == 0) return;
    volatile uint8_t i = delays;
    while(i--);
}

void clk(){
    sck.clr();
    delayDur();
    sck.set();
    delayDur();
}

Error start(const uint8_t _address) {
    occupied = _address & 0b1;
    ws.outpp();

    return ErrorType::OK;
}

void stop() {
    sda.clr();
    occupied = -1;
}


protected :
    Error begin_use(const uint8_t index = 0) override {
        return start(index);
    }
    void end_use() override {stop();}

    bool is_idle() override {
        return (occupied >= 0 ? false : true);
    }

    bool owned_by(const uint8_t index = 0) override{
        return (index == occupied);
    }

public:

    I2sSw(hal::GpioIntf & _sck,hal::GpioIntf & _sda,hal::GpioIntf & _ws,const uint16_t _delays = 10):sck(_sck), sda(_sda), ws(_ws), delays(_delays){;}

    Error write(const uint32_t data) override {
        sck.outpp();
        sda.outpp();

        uint16_t left_data = data >> 16;
        uint16_t rdata = data;

        ws = false;

        for(uint16_t mask = 0x8000; mask; mask >>= 1)
        {
            sck.clr();
            sda = rdata & mask;
            sck.set();
        }


        ws = true;
        for(uint16_t mask = 0x8000; mask; mask >>= 1)
        {
            sck.clr();
            sda = left_data & mask;
            sck.set();
        }

        ws = false;
        return ErrorType::OK;
    }

    Error read(uint32_t & data, bool toAck = true) {
        return ErrorType::OK;
    }

    Error transfer(uint32_t & data_rx, const uint32_t data_tx, bool toAck){
        return ErrorType::OK;
    }

    void init(const uint32_t baudRate){
        sck.outpp();
        sda.outpp();
        ws.outpp();
    }
    void setBaudRate(const uint32_t baudRate) override {;}
};

};