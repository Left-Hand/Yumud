/*
 * i2c-soft.h
 *
 *  Created on: Jun 9, 2013
 *      Author: agu
 */

#ifndef I2C_SOFT_H_
#define I2C_SOFT_H_

#include "i2c.hpp"
#include "../../gpio/gpio.hpp"

class I2cSw: public I2c{
private:
    
Gpio & scl;
Gpio & sda;
int8_t occupied = -1;
uint8_t delays = 0;
__fast_inline void delayDur(){
    // delayMicroseconds(1);
    __nopn(6);
    // for(volatile uint16_t i = 0; i < delays; i++);
}

void clk(){
    // delayDur();
    scl = true;
    delayDur();
    // delayDur();
    scl = false;
    // delayDur();
}

void ack(){
    delayDur();
    sda = false;
    delayDur();
    scl = true;
    delayDur();
    scl = false;
    delayDur();
}

void nack(void) {
    delayDur();
    sda = true;
    delayDur();
    scl = true;
    delayDur();
    scl = false;
    delayDur();
}

bool wait_ack(){
    bool ret;
    delayDur();
    scl = true;
    delayDur();
    ret = sda.read();
    scl = false;
    delayDur();
    return ret;
}

__fast_inline void start(const uint8_t & _address) {
    occupied = _address & 0x7F;
    sda = true;
    scl = true;
    delayDur();
    sda = false;
    delayDur();
    scl = false;
    delayDur();
    write(_address);
}

__fast_inline void stop() {
    delayDur();
    sda = false;
    delayDur();
    scl = true;
    delayDur();
    sda = true;
    delayDur();
    occupied = -1;
}


protected :
    void begin_use(const uint8_t & index = 0) override {start(index);}
    void end_use() override {stop();}

    bool is_idle() override {
        return (occupied >= 0 ? false : true);
    }

    bool owned_by(const uint8_t & index = 0) override{
        return (occupied == index);
    }

public:

    I2cSw(Gpio & _scl,Gpio & _sda):scl(_scl), sda(_sda){;};

    __fast_inline Error write(const uint32_t & data) override {
        sda.write(0x80 & data);
        clk();
        sda.write(0x40 & data);
        clk();
        sda.write(0x20 & data);
        clk();
        sda.write(0x10 & data);
        clk();
        
        sda.write(0x08 & data);
        clk();
        sda.write(0x04 & data);
        clk();
        sda.write(0x02 & data);
        clk();
        sda.write(0x01 & data);
        clk();

        wait_ack();

        return Bus::ErrorType::OK;
    }

    __fast_inline Error read(uint32_t & data, bool toAck = true) {
        uint8_t ret = 0;
        sda.InFloating();
        delayDur();
        ret |= sda.read();
        clk();
        ret <<= 1; ret |= sda.read(); 
        clk();
        ret <<= 1; ret |= sda.read(); 
        clk();
        ret <<= 1; ret |= sda.read(); 
        clk();
        ret <<= 1; ret |= sda.read(); 

        clk();
        ret <<= 1; ret |= sda.read(); 
        clk();
        ret <<= 1; ret |= sda.read(); 
        clk();
        ret <<= 1; ret |= sda.read(); 
        clk();

        if(toAck) ack();
        else nack();
        sda.OutPP();
        data = ret;

        return Bus::ErrorType::OK;
    }

    __fast_inline Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck){
        write(data_tx);
        read(data_rx, toAck);
        return ErrorType::OK;
    }

    void init(const uint32_t & baudRate) override {;}
    void configDataSize(const uint8_t & data_size) override {;}
    void configBaudRate(const uint32_t & baudRate) override {;}
    void configBitOrder(const bool & msb) override {;}
};

#endif 
