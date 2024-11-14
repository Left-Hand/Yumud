#include "hx711.hpp"


using namespace ymd::drivers;

uint32_t HX711::read_data(void){
    uint32_t data=0;

    for(uint8_t i = 0; i < 24; i++){
        sck_pin = true;
        __nopn(2);
        sck_pin = false;

        data <<= 1; data |= bool(sdo_pin);
    }

    for(uint8_t i = 0; i < (uint8_t)conv_type; i++){
        sck_pin = true;
        __nopn(2);
        sck_pin = false;
    }

    data ^= 0x800000;
    return(data);
}