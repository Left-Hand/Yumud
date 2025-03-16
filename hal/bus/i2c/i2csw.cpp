#include "i2csw.hpp"
#include "sys/clock/time_stamp.hpp"

using namespace ymd;
using namespace ymd::hal;

#define SCL_PP

void I2cSw::delayDur(){
    if(delays_) delayMicroseconds(delays_);
    // else __nopn(7);
}

BusError I2cSw::wait_ack(){
    delayDur();
    sda_gpio.set();
    sda_gpio.inflt();
    delayDur();
    scl_gpio.set();
    TimeStamp delta;

    bool ovt = false;
    while(sda_gpio.read()){
        if(delta >= timeout_){
            ovt = true;
            break;
        }
    }

    delayDur();
    scl_gpio.clr();
    delayDur();
    
    if(ovt){
        return BusError::NO_ACK;
    }else{
        return BusError::OK;
    }

}

BusError I2cSw::lead(const uint8_t address){
    #ifdef SCL_PP
    scl_gpio.outpp();
    #else
    scl_gpio.outod();
    #endif
    sda_gpio.outod();
    sda_gpio.set();
    scl_gpio.set();
    delayDur();
    sda_gpio.clr();
    delayDur();
    scl_gpio.clr();
    delayDur();
    return write(address);
}

void I2cSw::trail(){
    scl_gpio.clr();
    sda_gpio.outod();
    sda_gpio.clr();
    delayDur();
    scl_gpio.set();
    delayDur();
    sda_gpio.set();
    delayDur();
}



BusError I2cSw::write(const uint32_t data){

    sda_gpio.outod();

    for(uint8_t mask = 0x80; mask; mask >>= 1){
        sda_gpio.write(mask & data);
        delayDur();
        scl_gpio.set();
        delayDur();
        scl_gpio.clr();
    }

    return wait_ack();
}

BusError I2cSw::read(uint32_t & data, const Ack ack){
    uint8_t ret = 0;

    sda_gpio.set();
    sda_gpio.inpu();
    delayDur();

    for(uint8_t i = 0; i < 8; i++){
        scl_gpio.set();
        ret <<= 1; ret |= sda_gpio.read();
        delayDur();
        scl_gpio.clr();
        delayDur();
    }

    sda_gpio.write(!bool(ack));
    sda_gpio.outod();
    scl_gpio.set();
    delayDur();

    scl_gpio.clr();
    sda_gpio.inpu();

    data = ret;
    return BusError::OK;
}

void I2cSw::init(const uint32_t baudRate){

    sda_gpio.set();
    sda_gpio.outod();
    scl_gpio.set();

    #ifdef SCL_PP
    scl_gpio.outpp();
    #else
    scl_gpio.outod();
    #endif

    setBaudRate(baudRate);
}

void I2cSw::setBaudRate(const uint32_t baudRate) {
    if(baudRate == 0){
        delays_ = 0;
    }else{
        uint32_t b = baudRate / 1000;
        delays_ = 400 / b;
    }
}

BusError I2cSw::reset(){
    return BusError::OK;
}

BusError I2cSw::unlock_bus(){
    return BusError::OK;
}