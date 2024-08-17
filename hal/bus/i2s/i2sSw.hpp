/*
 * I2s-soft.h
 *
 *  Created on: Jun 9, 2013
 *      Author: agu
 */

#ifndef I2s_SOFT_H_
#define I2s_SOFT_H_

#include "I2s.hpp"
#include "../../gpio/gpio.hpp"

class I2sSw: public I2s{
private:
volatile int8_t occupied = -1;
GpioConcept & sck;
GpioConcept & sda;
GpioConcept & ws;

uint16_t delays = 0;

__fast_inline volatile void delayDur(){
    // volatile uint8_t i = delays;
    // while(i--);
}

void clk(){
    sck = false;
    delayDur();
    sck = true;
    delayDur();
}

Error start(const uint8_t & _address) {
    occupied = _address & 0b1;
    ws.outpp();

    return ErrorType::OK;
}

void stop() {
    sda = false;
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

    I2sSw(GpioConcept & _sck,GpioConcept & _sda,GpioConcept & _ws,const uint16_t & _delays = 10):sck(_sck), sda(_sda), ws(_ws), delays(_delays){;}

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
        return Bus::ErrorType::OK;
    }

    Error read(uint32_t & data, bool toAck = true) {
        return ErrorType::OK;
    }

    Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck){
        return ErrorType::OK;
    }

    void init(const uint32_t baudRate){
        sck.outpp();
        sda.outpp();
        ws.outpp();
    }
    void configBaudRate(const uint32_t baudRate) override {;}
};

#endif 
