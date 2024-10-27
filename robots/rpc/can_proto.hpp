#pragma once

#include "sys/math/float/bf16.hpp"
#include "hal/bus/can/can.hpp"

#include <tuple>

class FOCMotor;


class CanProtocolConcept{
public:

    using E = real_t;
    using E_1 = std::tuple<E>;
    using E_2 = std::tuple<E, E>;
    using E_3 = std::tuple<bf16, bf16, bf16>;
    using E_4 = std::tuple<bf16, bf16, bf16, bf16>;
public:
    Can & can;
    
    CanProtocolConcept(Can & _can):can(_can){;}

    virtual void parseCanmsg(const CanMsg & msg) = 0;


    void update(){
        if(can.available()){
            const CanMsg & msg = can.read();
            // uint8_t id = msg.id() >> 7;
            // if(id == 0 || id == uint8_t(node_id)){
            parseCanmsg(msg);
            // }
        }
    }

private:
    void collect(){

    }
};