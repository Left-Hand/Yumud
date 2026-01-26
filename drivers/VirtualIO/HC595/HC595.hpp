#pragma once

#include "hal/gpio/gpio.hpp"
#include "hal/gpio/vport.hpp"
#include <array>


namespace ymd::drivers{

class HC595_Transport final{
    HC595_Transport(
        hal::Gpio & sclk_io, 
        hal::Gpio & data_io, 
        hal::Gpio & latch_io
    ):
        sclk_io_(sclk_io), 
        data_io_(data_io), 
        latch_io_(latch_io)
    {;}

    void init(){
        sclk_io_.outpp();
        data_io_.outpp();
        latch_io_.outpp();
    }

    void write(const uint8_t data){
        latch_io_.clr();
        for(uint8_t mask = 0x80; mask; mask >>= 1){
            data_io_.write(mask & data);
            pulse();
        }
        latch_io_.set();
    }

private:
    hal::Gpio & sclk_io_;
    hal::Gpio & data_io_;
    hal::Gpio & latch_io_;

    void pulse(){
        sclk_io_.set();
        sclk_io_.clr();
    }


};

template<uint8_t N>
class HC595 final{
protected:
    HC595_Transport transport_;
    std::array<uint8_t, N> buf_;
    void flush(){
        for(int8_t i = 0; i < N; i++){
            write(buf_[i]);
        }
    }
public:
    HC595(HC595_Transport && phy):transport_(std::move(phy)){;}

    void init(){
        sclk_io_.outpp();
        data_io_.outpp();
        latch_io_.outpp();
    }
    void set_content(const std::span<const uint8_t, N> data_list){
        std::copy(data_list.begin(), data_list.end(), buf_.begin());
        flush();
    }

    HC595 & operator << (const uint8_t data){write(data); return *this;}
};

// class HC595Single: public VGpioPortIntf<8>{
// protected:
//     hal::Gpio & sclk_io_;
//     hal::Gpio & data_io_;
//     hal::Gpio & latch_io_;

//     uint8_t buf_ = 0;

//     void write(const uint16_t data) override{
//         buf_ = data;
//         latch_io_.clr();
//         for(uint8_t mask = 0x80; mask; mask >>= 1){
//             data_io_.write(mask & buf_);
//             sclk_io_.set();
//             sclk_io_.clr();
//         }
//         latch_io_.set();
//     }

//     uint16_t read() override{
//         return buf_;
//     }

// public:
//     HC595Single(hal::Gpio & sclk_io, hal::Gpio & data_io, hal::Gpio & latch_io):
//             sclk_io_(sclk_io), data_io_(data_io), latch_io_(latch_io){;}

//     void init(){
//         sclk_io_.outpp();
//         data_io_.outpp();
//         latch_io_.outpp();
//     }


//     void set(const PinSource pin) override{
//         if((uint8_t)pin == 0)return;
//         write(buf_ | (uint8_t)pin);
//     }
//     void clr(const PinSource pin) override{
//         if((uint8_t)pin == 0)return;
//         write(buf_ &= ~(uint8_t)(pin));
//     }

//     void set(const uint16_t data) override{
//         write(buf | (uint8_t)data);
//     }
//     void clr(const uint16_t data) override{
//         write(buf & (~(uint8_t)(data)));
//     }

//     void write_nth(const int index, const bool data) override{
//         if(index < 0 || index >= 8) return;
//         uint8_t last_buf = buf;
//         buf &= ~(uint8_t)(1 << index);
//         buf |= (uint8_t)(data << index);
//         if(last_buf != buf) write(buf);
//     }
//     bool read_nth(const int index) override{
//         if(index < 0 || index >= 8) return false;
//         return buf & (1 << index);
//     }


//     HC595Single & operator << (const uint8_t data){write(data); return *this;}
//     HC595Single & operator = (const uint16_t data) override {write(data); return *this;}
// };

}