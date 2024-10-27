#pragma once

#include "zaxis.hpp"


class FOCMotorConcept;

namespace gxm{
class ZAxisStepper:public ZAxis{
protected:
    using Motor = FOCMotorConcept;
    // Motor & motor_;
public:
    // ZAxisStepper(const Config & config, Motor & motor):ZAxis(config), motor_(motor){}
    ZAxisStepper(const Config & config):ZAxis(config){}
    DELETE_COPY_AND_MOVE(ZAxisStepper)
    
    bool reached() override;
    void setDistance(const real_t dist) override;  
    void tick() override;

    void softHome() override;
};


}