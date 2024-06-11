#ifndef __LED_HPP__

#define __LED_HPP__

#include "hal/gpio/gpio.hpp"

template<bool com_anode>
class Led{
protected:
    bool state = false;
    GpioConcept & instance;
public:
    Led(GpioConcept & _instance):instance(_instance){;}
    Led & operator = (const bool & _state){
        state = _state;
        instance = state ^ com_anode;
        return *this;
    }
    operator bool() const{return state;}
};

#endif