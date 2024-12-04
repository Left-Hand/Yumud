#pragma once

#include "sys/math/float/bf16.hpp"
#include "hal/bus/can/can.hpp"

#include <tuple>

// class FOCMotor;

namespace ymd{

class CanProtocolConcept{
public:

    using E = real_t;
    using E_1 = std::tuple<E>;
    using E_2 = std::tuple<E, E>;
    using E_3 = std::tuple<bf16, bf16, bf16>;
    using E_4 = std::tuple<bf16, bf16, bf16, bf16>;

public:
    Can & can_;
    uint8_t node_id_;

    CanProtocolConcept(
        Can & _can, const uint8_t node_id):
        can_(_can), node_id_(node_id){;}

    virtual void parseCanmsg(const CanMsg & msg) = 0;


    bool update(const CanMsg & msg){
        // if(can_.available()){
            // const CanMsg msg = can_.read();
            // uint8_t id = msg.id() >> 7;
            // if(id == 0 || id == uint8_t(node_id)){

        if(msg.id() >> 7 == node_id_) return false;
        parseCanmsg(msg);
        return true;
    }

    Can & can(){
        return can_;
    }
private:
    void collect(){

    }
};

}