#include "TTP229.hpp"
#include "hal/gpio/gpio_intf.hpp"

using namespace ymd::drivers;

void TTP229::update(){
    uint16_t new_map = 0;
    sck_pin_.set_low();

    for (size_t i = 0; i < 16; i++){
        sck_pin_.set_high();
        sck_pin_.set_low();
        new_map <<= 1; 
        new_map |= sdo_pin_.read().to_bool();
    }

    map_ = new_map;
    uint8_t new_num = 0;
    while(new_map != (1 << new_num)){
        new_num++;
        if(new_num >= 16){
            num_ = None;
            return;
        }
    }
    num_ = Some(new_num);
}

bool TTP229::is_idle(){
    return sdo_pin_.read() == LOW;
}