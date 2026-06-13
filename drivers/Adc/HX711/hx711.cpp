#include "hx711.hpp"
#include "hal/gpio/gpio_intf.hpp"


using namespace ymd::drivers;

uint32_t HX711_Transport::read_data(const HX711_Prelude::ConvType conv_type){
    uint32_t bits=0;

    for(size_t i = 0; i < 24; i++){
        sck_pin.set_high();
        __nopn(2);
        sck_pin.set_low();

        bits <<= 1; 
        bits |= bool(sdo_pin.read() == HIGH);
    }

    for(size_t i = 0; i < static_cast<uint8_t>(conv_type); i++){
        sck_pin.set_high();
        __nopn(2);
        sck_pin.set_low();
    }

    return(bits ^ 0x800000);
}

void HX711::init(){
    transport_.sck_pin.outpp();
    transport_.sdo_pin.inpu();
    read_data();
}

bool HX711::is_idle(){
    return (transport_.sdo_pin.read() == LOW);
}