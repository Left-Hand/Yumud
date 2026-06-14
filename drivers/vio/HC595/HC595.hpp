#pragma once

#include "hal/gpio/gpio.hpp"
#include "hal/gpio/vport.hpp"
#include <array>


namespace ymd::drivers{

class HC595_Transport final{
    HC595_Transport(
        hal::Gpio & sclk_pin, 
        hal::Gpio & data_pin, 
        hal::Gpio & latch_pin
    ):
        sclk_pin_(sclk_pin), 
        data_pin_(data_pin), 
        latch_pin_(latch_pin)
    {;}

    void init(){
        sclk_pin_.outpp();
        data_pin_.outpp();
        latch_pin_.outpp();
    }

    void write_byte(const uint8_t byte){
        latch_pin_.set_low();
        for(uint8_t mask = 0x80; mask; mask >>= 1){
            data_pin_.write(BoolLevel::from(mask & byte));
            pulse();
        }
        latch_pin_.set_high();
    }

private:
    hal::Gpio & sclk_pin_;
    hal::Gpio & data_pin_;
    hal::Gpio & latch_pin_;

    void pulse(){
        sclk_pin_.set_high();
        sclk_pin_.set_low();
    }


};


}