#ifndef __HC595_HPP__

#define __HC595_HPP__

#include "src/gpio/gpio.hpp"
#include "src/gpio/port_virtual.hpp"
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

    HC595 & operator << (const uint8_t & data){write(data); return *this;}
};

class HC595Single: public PortVirtualInst<8>{
protected:
    GpioBase & sclk_pin;
    GpioBase & data_pin;
    GpioBase & latch_pin;

    uint8_t buf = 0;

    void write(const uint16_t & data) override{
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
    HC595Single(GpioBase & _sclk_pin, GpioBase & _data_pin, GpioBase & _latch_pin):
            sclk_pin(_sclk_pin), data_pin(_data_pin), latch_pin(_latch_pin){;}

    void init(){
        sclk_pin.OutPP();
        data_pin.OutPP();
        latch_pin.OutPP();
    }


    void set(const Pin & pin) override{
        if((uint8_t)pin == 0)return;
        write(buf | (uint8_t)pin);
    }
    void clr(const Pin & pin) override{
        if((uint8_t)pin == 0)return;
        write(buf &= ~(uint8_t)(pin));
    }

    void setBits(const uint16_t & data) override{
        write(buf | (uint8_t)data);
    }
    void clrBits(const uint16_t & data) override{
        write(buf & (~(uint8_t)(data)));
    }

    void writeByIndex(const int8_t index, const bool data) override{
        if(index < 0 || index >= 8) return;
        uint8_t last_buf = buf;
        buf &= ~(uint8_t)(1 << index);
        buf |= (uint8_t)(data << index);
        if(last_buf != buf) write(buf);
    }
    bool readByIndex(const int8_t index) override{
        if(index < 0 || index >= 8) return false;
        return buf & (1 << index);
    }

    void setModeByIndex(const int8_t & index, const PinMode & mode){}

    HC595Single & operator << (const uint8_t & data){write(data); return *this;}
    HC595Single & operator = (const uint16_t & data) override {write(data); return *this;}
};


#endif