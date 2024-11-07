#include "TTP229.hpp"

using namespace yumud::drivers;

void TTP229::scan(){
    uint16_t new_map = 0;
    sck_pin = false;

    for (int i = 0; i < 16; i++)
    {
        sck_pin = true;
        sck_pin = false;
        new_map <<= 1; new_map |= sdo_pin.read();
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