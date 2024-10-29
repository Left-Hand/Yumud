#pragma once

#include "../hal/gpio/gpio.hpp"

class TTP229{
protected:
    int8_t num = -1;
    uint16_t map = 0;
    GpioConcept & sck_pin;
    GpioConcept & sdo_pin;

public:
    TTP229(GpioConcept & _sck_pin, GpioConcept & _sdo_pin):sck_pin(_sck_pin), sdo_pin(_sdo_pin){;}
    ~TTP229(){;}
    void scan(){
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

    bool hasKey(){return (bool)(num >= 0);}
    bool isIdle(){return sdo_pin == false;}
    int8_t getNum(){return num+1;}
    int8_t getONum(){return num;}
    void getMapData(uint16_t & _map){_map = map;}
};

