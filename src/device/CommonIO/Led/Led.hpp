#ifndef __LED_HPP__

#define __LED_HPP__

#include "src/gpio/gpio.hpp"

template<bool com_anode>
class Led{
protected:
    bool state = false;
    GpioBase & instance;
public:
    Led(GpioBase & _instance):instance(_instance){;}
    operator = (const bool & _state){
        state = _state;
        instance = state ^ com_anode;
        return *this;
    }
    operator bool() const{return state;}
};

#endif