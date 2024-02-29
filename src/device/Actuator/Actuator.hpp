#ifndef __ACTUATOR_HPP__

#define __ACTUATOR_HPP__

#include "real.hpp"
#include "functional"

class Actuator{
protected:
    std::function<void (real_t)> speed_setter;
    std::function<real_t> speed_getter;
    std::function<void(real_t)> position_setter;
    std::function<real_t()> position_getter;

    Actuator(
            std::function<void (real_t)> _speed_setter,
            std::function<real_t> _speed_getter,
            std::function<void(real_t)> _position_setter,
            std::function<real_t()> _position_getter):
            speed_setter(_speed_setter),
            speed_getter(_speed_getter),
            position_setter(_position_setter),
            position_getter(_position_getter){;}
public:
    void setSpeed(real_t speed){
        if(speed_setter) speed_setter(speed);
    }

    real_t getSpeed(){
        return speed_getter();
    }

    void setPosition(real_t position){
        if(position_setter) position_setter(position);
    }

    real_t getPosition(){
        return position_getter();
    }
};

#endif