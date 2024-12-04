#pragma once

#include "hal/bus/can/can.hpp"
#include "robots/rpc/can_proto.hpp"

namespace gxm{

class CanMaster{
    using CanProtocolConcept = ymd::CanProtocolConcept;
    using Wrapper = std::reference_wrapper<CanProtocolConcept>;
    using Container = std::vector<Wrapper>;

    Container pool = {};
public:
    CanMaster(CanMaster && other) = delete;
    CanMaster(CanMaster & other) = delete;

    void registerNode(CanProtocolConcept & proto){
        pool.push_back(proto);
    }

    void update();
};


}