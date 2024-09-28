#pragma once

#include "sys/math/float/bf16.hpp"
#include "hal/bus/can/can.hpp"

#include "robots/foc/stepper/motor_utils.hpp"
#include <tuple>

class FOCMotor;


class CanProtocolConcept{
public:
    using E = bf16;
    using E_2 = std::tuple<E, E>;
    using E_3 = std::tuple<E, E, E>;
    using E_4 = std::tuple<E, E, E, E>;
    
    template<integral T>
    struct NodeId_t{
        T id_;

        NodeId_t(const T _id):id_(_id){}
        operator T() const{return id_;}
    };

    using NodeId =  NodeId_t<uint8_t>;
public:
    Can & can;
    NodeId node_id;
    
    CanProtocolConcept(Can & _can):
        can(_can), node_id(getDefaultNodeId()){;}

    CanProtocolConcept(Can & _can, const NodeId _node_id):
        can(_can), node_id(_node_id){;}

    virtual void parseCanmsg(const CanMsg & msg) = 0;
    void update(){collect();}


private:
    void collect(){
        if(can.available()){
            const CanMsg & msg = can.read();
            uint8_t id = msg.id() >> 7;
            if(id == 0 || id == uint8_t(node_id)){
                parseCanmsg(msg);
            }
        }
    }

    NodeId getDefaultNodeId();
};