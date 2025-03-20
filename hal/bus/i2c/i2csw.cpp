#include "i2csw.hpp"
#include "core/clock/time_stamp.hpp"
#include "hal/gpio/gpio.hpp"

using namespace ymd;
using namespace ymd::hal;

#define SCL_PP

void I2cSw::delay_dur(){
    if(delays_) delayMicroseconds(delays_);
    // else __nopn(7);
}

BusError I2cSw::wait_ack(){
    delay_dur();
    sda_gpio.set();
    sda_gpio.inflt();
    delay_dur();
    scl_gpio.set();
    TimeStamp delta;

    bool ovt = false;
    while(sda_gpio.read()){
        if(delta.duration() >= timeout_){
            ovt = true;
            break;
        }
    }

    delay_dur();
    scl_gpio.clr();
    delay_dur();
    
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
    delay_dur();
    sda_gpio.clr();
    delay_dur();
    scl_gpio.clr();
    delay_dur();
    return write(address);
}

void I2cSw::trail(){
    scl_gpio.clr();
    sda_gpio.outod();
    sda_gpio.clr();
    delay_dur();
    scl_gpio.set();
    delay_dur();
    sda_gpio.set();
    delay_dur();
}



BusError I2cSw::write(const uint32_t data){

    sda_gpio.outod();

    for(uint8_t mask = 0x80; mask; mask >>= 1){
        sda_gpio.write(mask & data);
        delay_dur();
        scl_gpio.set();
        delay_dur();
        scl_gpio.clr();
    }

    return wait_ack();
}

BusError I2cSw::read(uint32_t & data, const Ack ack){
    uint8_t ret = 0;

    sda_gpio.set();
    sda_gpio.inpu();
    delay_dur();

    for(uint8_t i = 0; i < 8; i++){
        scl_gpio.set();
        ret <<= 1; ret |= sda_gpio.read();
        delay_dur();
        scl_gpio.clr();
        delay_dur();
    }

    sda_gpio.write(!bool(ack));
    sda_gpio.outod();
    scl_gpio.set();
    delay_dur();

    scl_gpio.clr();
    sda_gpio.inpu();

    data = ret;
    return BusError::OK;
}

void I2cSw::init(const uint32_t baudrate){

    sda_gpio.set();
    sda_gpio.outod();
    scl_gpio.set();

    #ifdef SCL_PP
    scl_gpio.outpp();
    #else
    scl_gpio.outod();
    #endif

    set_baudrate(baudrate);
}

void I2cSw::set_baudrate(const uint32_t baudrate) {
    if(baudrate == 0){
        delays_ = 0;
    }else{
        uint32_t b = baudrate / 1000;
        delays_ = 400 / b;
    }
}

BusError I2cSw::reset(){
    return BusError::OK;
}

BusError I2cSw::unlock_bus(){
    return BusError::OK;
}