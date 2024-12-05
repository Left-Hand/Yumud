#pragma once

#include "hal/bus/can/can.hpp"
#include "robots/rpc/can_proto.hpp"

namespace gxm{

class CanMaster{
protected:
    using Can = ymd::Can;
    using CanProtocolConcept = ymd::CanProtocolConcept;
    // using Wrapper = (CanProtocolConcept *);
    using Container = std::vector<CanProtocolConcept *>;

    Container pool = {};
    Can & can_;
public:
    CanMaster(Can & can):can_(can){;}
    CanMaster(CanMaster && other) = delete;
    CanMaster(CanMaster & other) = delete;

    void registerNode(CanProtocolConcept & proto){
        pool.push_back(&proto);
    }

    // void registerNodes(CanProtocolConcept * begin, CanProtocolConcept * end){
    //     int dist = end - begin;
    //     if(dist < 0) HALT;

    //     for(size_t i = 0; i < size_t(dist); i++){
    //         registerNode(begin[i]);
    //     }
    // }

    void update();
};


}