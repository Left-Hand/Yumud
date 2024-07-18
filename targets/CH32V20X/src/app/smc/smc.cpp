#include "smc.h"

namespace SMC{


ElementHolder::ElementHolder(SmartCar & _owner):owner(_owner){;}


void ElementHolder::invoke(){

    __disable_irq();
    owner.switches.element_type = next_element_type;

    __enable_irq();
    DEBUG_PRINTLN("sw ele", next_element_type, next_element_side, next_element_status);
}

void ElementHolder::update(){
    m_locker.update();
    if(bool(m_locker) == false){
        invoke();
    }
}

void ElementHolder::request(const ElementType new_element_type, const uint8_t new_element_status, const LR new_element_side, const AlignMode, const ElementLocker & locker){
    if(new_element_type != next_element_type){
        next_element_type = new_element_type;
        next_element_side = new_element_side;
        next_element_status = new_element_status;

        if(bool(locker) == false){
            invoke();
        }else{
            m_locker = locker;
            m_locker.init();
        }
    }
}

ElementLocker::ElementLocker(SmartCar & _owner):owner(&_owner), remain_time(0), remain_travel(0){;}


void ElementLocker::init(){
    last_t = t;
    last_travel = owner->measurer.get_travel();
}

void ElementLocker::update(){
    auto delta_t = t - last_t;
    last_t = t;

    remain_time -= delta_t;

    if(owner){
        auto now_travel = owner->measurer.get_travel();
        auto delta_travel = now_travel - last_travel;
        last_travel = now_travel;

        remain_travel -= delta_travel;
    }
}


ElementLocker::ElementLocker(const ElementLocker & other){
    owner = other.owner;
    last_t = other.last_t;
    last_travel = other.last_travel;
    remain_time = other.remain_time;
    remain_travel = other.remain_travel;
}

ElementLocker::ElementLocker(SmartCar &_owner, const real_t lasting_time, const real_t lasting_travel)
{
    owner = &_owner;
    last_t = t;
    last_travel = owner->measurer.get_travel();
    remain_time = lasting_time;
    remain_travel = lasting_travel;
}

}