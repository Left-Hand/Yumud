#include "i2csw.hpp"
#include "sys/clock/time_stamp.hpp"

I2cSw::Error I2cSw::wait_ack(){
    sda_gpio.set();
    sda_gpio.inflt();
    delayDur();
    scl_gpio.set();
    TimeStamp stamp;

    bool overtime = false;
    while(sda_gpio.read()){
        if(stamp >= timeout){
            overtime = true;
            break;
        }
    }

    if(overtime){
        delayDur();
        scl_gpio.clr();
        delayDur();
        return ErrorType::TIMEOUT;
    }else{
        delayDur();
        scl_gpio.clr();
        delayDur();
        return ErrorType::OK;
    }

}

I2cSw::Error I2cSw::lead(const uint8_t _address){
    scl_gpio.outod();
    sda_gpio.outod();
    sda_gpio.set();
    scl_gpio.set();
    delayDur();
    sda_gpio.clr();
    delayDur();
    scl_gpio.clr();
    delayDur();
    write(_address);

    return ErrorType::OK;
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



I2cSw::Error I2cSw::write(const uint32_t data){
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

I2cSw::Error I2cSw::read(uint32_t & data, const bool toAck){
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

    sda_gpio.write(!toAck);
    sda_gpio.outod();
    scl_gpio.set();
    delayDur();
    scl_gpio.clr();
    sda_gpio.inpu();

    data = ret;
    return I2cSw::ErrorType::OK;
}

void I2cSw::init(const uint32_t baudRate){
    // preinit();

    sda_gpio.set();
    sda_gpio.outod();
    scl_gpio.set();
    scl_gpio.outod();

    setBaudRate(baudRate);
}

void I2cSw::setBaudRate(const uint32_t baudRate) {
    if(baudRate == 0){
        delays = 0;
    }else{
        uint32_t b = baudRate / 1000;
        delays = 400 / b;
    }
}