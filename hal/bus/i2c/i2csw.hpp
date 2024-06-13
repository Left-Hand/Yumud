#ifndef I2C_SOFT_H_
#define I2C_SOFT_H_

#include "i2c.hpp"
#include "hal/gpio/gpio.hpp"
#include "src/clock/time_stamp.hpp"

class I2cSw: public I2c{
private:
    volatile int8_t occupied = -1;
    GpioConcept & scl;
    GpioConcept & sda;

    uint16_t delays = 10;

    __fast_inline void delayDur(){
        delayMicroseconds(delays);
    }


    Error wait_ack(){
        sda.set();
        sda.InFloating();
        delayDur();
        scl.set();
        TimeStamp stamp;
        while(sda.read()){
            if(stamp >= timeout){
                delayDur();
                scl.clr();
                return ErrorType::TIMEOUT;
            }
        }
        delayDur();
        scl.clr();
        delayDur();
        return ErrorType::OK;
    }

    Error lead(const uint8_t _address) override{
        scl.OutOD();
        sda.OutOD();
        sda.set();
        scl.set();
        delayDur();
        sda.clr();
        delayDur();
        scl.clr();
        delayDur();
        write(_address);

        return ErrorType::OK;
    }

    void trail() override {
        scl.clr();
        sda.OutOD();
        sda.clr();
        delayDur();
        scl.set();
        delayDur();
        sda.set();
        delayDur();
    }


protected :

    void reset() override {};
    void lock_avoid() override {};
public:

    I2cSw(GpioConcept & _scl,GpioConcept & _sda, const uint16_t & _delays = 10):scl(_scl), sda(_sda), delays(_delays){;}

    Error write(const uint32_t data) override {
        sda.OutOD();

        for(uint8_t mask = 0x80; mask; mask >>= 1){
            sda.write(mask & data);
            delayDur();
            scl.set();
            delayDur();
            scl.clr();
        }

        return wait_ack();
    }

    __fast_inline Error read(uint32_t & data, bool toAck = true) {
        uint8_t ret = 0;

        sda.set();
        sda.InPullUP();
        delayDur();

        for(uint8_t i = 0; i < 8; i++){
            scl.set();
            ret <<= 1; ret |= sda.read();
            delayDur();
            scl.clr();
            delayDur();
        }

        sda.write(!toAck);
        sda.OutOD();
        scl.set();
        delayDur();
        scl.clr();
        sda.InPullUP();

        data = ret;
        return Bus::ErrorType::OK;
    }

    Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck){
        return ErrorType::OK;
    }

    void init(const uint32_t baudRate){
        preinit();

        sda.set();
        sda.OutOD();
        scl.set();
        scl.OutOD();

        configBaudRate(baudRate);
    }

    void configBaudRate(const uint32_t baudRate) override {
        if(baudRate == 0){
            delays = 0;
        }else{
            uint32_t b = baudRate / 1000;
            delays = 400 / b;
        }
    }
};

#endif
