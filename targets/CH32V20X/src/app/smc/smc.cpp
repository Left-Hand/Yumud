#include "smc.h"

namespace SMC{


ElementHolder::ElementHolder(SmartCar & _owner):owner(_owner){;}


void ElementHolder::invoke(){

    __disable_irq();
    owner.switches.element_type = next_element_type;
    owner.switches.element_status = next_element_status;
    owner.switches.element_side = next_element_side;
    owner.switches.align_mode = next_align_mode;
    __enable_irq();

}

void ElementHolder::update(){
    // auto travel =  owner.measurer.get_travel();
    // DEBUG_PRINTLN("updated");
    // if((invoked = false) && t >= unlock_t && travel >= unlock_travel){
    //     invoke();
    //     invoked = true;
    // }
}


bool ElementHolder::is_locked() const {
    auto travel =  owner.measurer.get_travel();
    return ((t < unlock_t) || (travel < unlock_travel));
}
void ElementHolder::reset(){
    invoked = false;

    unlock_t = 0;
    unlock_travel = 0;
    next_element_type = owner.switches.element_type;
    next_element_side = owner.switches.element_side;
    next_element_status = owner.switches.element_status;
}

void ElementHolder::request(const ElementType new_element_type, const uint8_t new_element_status, const LR new_element_side, const AlignMode align_mode, const ElementLocker & locker){
    auto travel =  owner.measurer.get_travel();
    if(is_locked()) return;

    if(             ((new_element_type != owner.switches.element_type)
                || ((uint8_t)new_element_status != owner.switches.element_status)
                || (new_element_side != owner.switches.element_side))){
        
        unlock_t = t + locker.remain_time;
        unlock_travel = travel + locker.remain_travel;
    
        owner.switches.element_type = new_element_type;
        owner.switches.element_side = new_element_side;
        owner.switches.element_status = new_element_status;
        owner.switches.align_mode = align_mode;

        DEBUG_PRINTLN("Ele!", new_element_type, new_element_side, align_mode, new_element_status);
        // invoked = false;
    }else{
        next_element_type = new_element_type;
        next_element_side = new_element_side;
        next_element_status = new_element_status;
        next_align_mode = align_mode;
    }
}

}