#pragma once

#include "core/math/float/bf16.hpp"
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
    hal::Can & can_;
    uint8_t node_id_;

    CanProtocolConcept(
        hal::Can & _can, const uint8_t node_id):
        can_(_can), node_id_(node_id){;}

    virtual void parseCanmsg(const hal::CanMsg & msg) = 0;


    bool update(const hal::CanMsg & msg){
        TODO();
        // auto node_id = msg.id() >> 7;
        // // if(node_id != node_id_ and node_id != 0) return false;
        // if(node_id != node_id_) return false;
        // parseCanmsg(msg);
        // return true;
    }

    hal::Can & can(){
        return can_;
    }
private:
    void collect(){

    }
};

}