#include "hx711.hpp"
#include "hal/gpio/gpio_intf.hpp"


using namespace ymd::drivers;

uint32_t HX711::read_data(void){
    uint32_t data=0;

    for(uint8_t i = 0; i < 24; i++){
        sck_gpio_.set();
        __nopn(2);
        sck_gpio_.clr();

        data <<= 1; data |= bool(sdo_gpio_.read() == HIGH);
    }

    for(uint8_t i = 0; i < (uint8_t)conv_type; i++){
        sck_gpio_.set();
        __nopn(2);
        sck_gpio_.clr();
    }

    data ^= 0x800000;
    return(data);
}

void HX711::init(){
    sck_gpio_.outpp();
    sdo_gpio_.inpu();
    read_data();
}

bool HX711::is_idle(){
    return (sdo_gpio_.read() == LOW);
}