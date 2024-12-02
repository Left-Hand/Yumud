#pragma once

#include "zaxis.hpp"



namespace ymd::foc{
class FOCMotorConcept;
}

namespace gxm{
class ZAxisStepper:public ZAxis{
protected:
    using Motor = ymd::foc::FOCMotorConcept;
    Motor & motor_;
public:
    ZAxisStepper(const Config & config, Motor & motor):ZAxis(config), motor_(motor){}
    DELETE_COPY_AND_MOVE(ZAxisStepper)
    
    bool reached() override;
    void setDistance(const real_t dist) override;  
    real_t getDistance() override{return 0;}
    void tick() override;

    void softHome() override;
};


}