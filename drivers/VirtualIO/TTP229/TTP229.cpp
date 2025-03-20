#include "TTP229.hpp"
#include "hal/gpio/gpio_intf.hpp"

using namespace ymd::drivers;

void TTP229::scan(){
    uint16_t new_map = 0;
    sck_gpio_ = false;

    for (int i = 0; i < 16; i++)
    {
        sck_gpio_ = true;
        sck_gpio_ = false;
        new_map <<= 1; new_map |= sdo_gpio_.read();
    }
    map = new_map;
    uint8_t new_num = 0;
    while(new_map != (1 << new_num)){
        new_num++;
        if(new_num >= 16){
            num = -1;
            return;
        }
    }
    num = new_num;
}

bool TTP229::isIdle(){return bool(sdo_gpio_) == false;}