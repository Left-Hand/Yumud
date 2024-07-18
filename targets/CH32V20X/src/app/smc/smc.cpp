#include "smc.h"

using namespace SMC;

void ElementHolder::update(){
    auto delta_t = t - last_t;
    last_t = t;

    remain_time = MAX(delta_t, 0);
    if(remain_time == 0 and next_element_type != ElementType::NONE){
        owner.sw_element(next_element_type, next_element_status, next_element_side);
    }
}

void ElementHolder::request(const ElementType new_element_type, const uint8_t new_element_status, const LR new_element_side, const real_t _remain_time){
    if(new_element_type != next_element_type){
        last_t = t;
        remain_time = _remain_time;
        next_element_type = new_element_type;
        next_element_side = new_element_side;
        next_element_status = new_element_status;

        if(_remain_time == 0){
            owner.sw_element(next_element_type, next_element_status, next_element_side);
        }
    }
}