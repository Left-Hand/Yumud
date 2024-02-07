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
GpioBase & sck;
GpioBase & sda;
GpioBase & ws;

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

void start(const uint8_t & _address) {
    occupied = _address & 0b1;
    ws.OutPP();
}

void stop() {
    sda = false;
    occupied = -1;
}


protected :
    void begin_use(const uint8_t & index = 0) override {start(index);}
    void end_use() override {stop();}

    bool is_idle() override {
        return (occupied >= 0 ? false : true);
    }

    bool owned_by(const uint8_t & index = 0) override{
        return (index == occupied);
    }

public:

    I2sSw(GpioBase & _sck,GpioBase & _sda,GpioBase & _ws,const uint16_t & _delays = 10):sck(_sck), sda(_sda), ws(_ws), delays(_delays){;}

    Error write(const uint32_t & data) override {
        sck.OutPP();
        sda.OutPP();

        uint16_t ldata = data >> 16;
        uint16_t rdata = data;

        ws = false;
        delayDur();

        for(uint8_t i=0; i<16; i++)
        {
            sck = false;
            if( (rdata>>(15-i))&0x0001 ) sda = true;
            else sda = false;
            delayDur();
            sck = true;
            delayDur();
        }

        delayDur();

        ws = true;
        delayDur();
        for(uint8_t i=0; i<16; i++)
        {
            sck = false;
            if( (ldata>>(15-i))&0x0001 ) sda = true;
            else sda = false;
            delayDur();
            sck = true;
            delayDur();
        }

        delayDur();
        ws = false;
        return Bus::ErrorType::OK;
    }

    Error read(uint32_t & data, bool toAck = true) {
        return ErrorType::OK;
    }

    Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck){
        return ErrorType::OK;
    }

    void init(const uint32_t & baudRate) override {;}
    void configDataSize(const uint8_t & data_size) override {;}
    void configBaudRate(const uint32_t & baudRate) override {;}
    void configBitOrder(const bool & msb) override {;}
};

#endif 
