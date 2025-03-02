#pragma once

#include "../hal/gpio/gpio.hpp"
#include "../hal/gpio/port_virtual.hpp"
#include <array>
#include <initializer_list>

template<uint8_t len>
class HC595{
protected:
    hal::GpioIntf & sclk_pin;
    hal::GpioIntf & data_pin;
    hal::GpioIntf & latch_pin;

    std::array<uint8_t, len> buf;

    void write(const uint8_t data){
        latch_pin.clr();
        for(uint8_t mask = 0x80; mask; mask >>= 1){
            data_pin.write(mask & data);
            sclk_pin.set();
            sclk_pin.clr();
        }
        latch_pin.set();
    }

    void flush(){
        for(int8_t i = 0; i < len; i++){
            write(buf[i]);
        }
    }
public:
    HC595(hal::GpioIntf & _sclk_pin, hal::GpioIntf & _data_pin, hal::GpioIntf & _latch_pin):
            sclk_pin(_sclk_pin), data_pin(_data_pin), latch_pin(_latch_pin){;}

    void init(){
        sclk_pin.outpp();
        data_pin.outpp();
        latch_pin.outpp();
    }
    void setContent(const uint8_t * data_ptr, size_t data_len){
        for(size_t i = 0; i < data_len; i++){
            buf[i] = data_ptr[i];
        }

        flush();
    }

    void setContent(const std::initializer_list<uint8_t> & data_list){
        uint8_t i = 0;
        for(const auto & data : data_list){
            buf[i] = data;
            i++;
        }

        flush();
    }

    HC595 & operator << (const uint8_t data){write(data); return *this;}
};

class HC595Single: public VGpioPortIntf<8>{
protected:
    hal::GpioIntf & sclk_pin;
    hal::GpioIntf & data_pin;
    hal::GpioIntf & latch_pin;

    uint8_t buf = 0;

    void write(const uint16_t data) override{
        buf = data;
        latch_pin.clr();
        for(uint8_t mask = 0x80; mask; mask >>= 1){
            data_pin.write(mask & buf);
            sclk_pin.set();
            sclk_pin.clr();
        }
        latch_pin.set();
    }

    uint16_t read() override{
        return buf;
    }

public:
    HC595Single(hal::GpioIntf & _sclk_pin, hal::GpioIntf & _data_pin, hal::GpioIntf & _latch_pin):
            sclk_pin(_sclk_pin), data_pin(_data_pin), latch_pin(_latch_pin){;}

    void init(){
        sclk_pin.outpp();
        data_pin.outpp();
        latch_pin.outpp();
    }


    void set(const Pin pin) override{
        if((uint8_t)pin == 0)return;
        write(buf | (uint8_t)pin);
    }
    void clr(const Pin pin) override{
        if((uint8_t)pin == 0)return;
        write(buf &= ~(uint8_t)(pin));
    }

    void set(const uint16_t data) override{
        write(buf | (uint8_t)data);
    }
    void clr(const uint16_t data) override{
        write(buf & (~(uint8_t)(data)));
    }

    void writeByIndex(const int index, const bool data) override{
        if(index < 0 || index >= 8) return;
        uint8_t last_buf = buf;
        buf &= ~(uint8_t)(1 << index);
        buf |= (uint8_t)(data << index);
        if(last_buf != buf) write(buf);
    }
    bool readByIndex(const int index) override{
        if(index < 0 || index >= 8) return false;
        return buf & (1 << index);
    }


    HC595Single & operator << (const uint8_t data){write(data); return *this;}
    HC595Single & operator = (const uint16_t data) override {write(data); return *this;}
};
