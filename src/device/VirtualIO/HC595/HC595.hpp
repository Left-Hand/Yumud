#ifndef __HC595_HPP__

#define __HC595_HPP__

#include "src/gpio/gpio.hpp"
#include <array>
#include <initializer_list>

template<uint8_t len>
class HC595{
protected:
    GpioBase & sclk_pin;
    GpioBase & data_pin;
    GpioBase & latch_pin;

    std::array<uint8_t, len> buf;

    void write(const uint8_t & data){
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
    HC595(GpioBase & _sclk_pin, GpioBase & _data_pin, GpioBase & _latch_pin):
            sclk_pin(_sclk_pin), data_pin(_data_pin), latch_pin(_latch_pin){;}

    void init(){
        sclk_pin.OutPP();
        data_pin.OutPP();
        latch_pin.OutPP();
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

    uint8_t & 
    HC595 & operator << (const uint8_t & data){write(data); return *this;}
};


#endif