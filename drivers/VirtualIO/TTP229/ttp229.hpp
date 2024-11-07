#pragma once

#include "hal/gpio/gpio.hpp"


namespace yumud::drivers{
class TTP229{
protected:
    int8_t num = -1;
    uint16_t map = 0;
    GpioConcept & sck_pin;
    GpioConcept & sdo_pin;

public:
    TTP229(GpioConcept & _sck_pin, GpioConcept & _sdo_pin):sck_pin(_sck_pin), sdo_pin(_sdo_pin){;}
    ~TTP229(){;}

    void scan();
    bool hasKey(){return (bool)(num >= 0);}
    bool isIdle(){return sdo_pin == false;}
    int8_t getNum(){return num+1;}
    int8_t getONum(){return num;}
    void getMapData(uint16_t & _map){_map = map;}
};

};