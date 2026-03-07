#include "hx711.hpp"
#include "hal/gpio/gpio_intf.hpp"


using namespace ymd::drivers;

uint32_t HX711::read_data(void){
    uint32_t bits=0;

    for(size_t i = 0; i < 24; i++){
        sck_pin_.set_high();
        __nopn(2);
        sck_pin_.set_low();

        bits <<= 1; 
        bits |= bool(sdo_pin_.read() == HIGH);
    }

    for(size_t i = 0; i < static_cast<uint8_t>(conv_type_); i++){
        sck_pin_.set_high();
        __nopn(2);
        sck_pin_.set_low();
    }

    return(bits ^ 0x800000);
}

void HX711::init(){
    sck_pin_.outpp();
    sdo_pin_.inpu();
    read_data();
}

bool HX711::is_idle(){
    return (sdo_pin_.read() == LOW);
}