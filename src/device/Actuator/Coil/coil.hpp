#ifndef __COIL_HPP__
#define __COIL_HPP__

#include "src/device/Actuator/Actuator.hpp"

class Coil:public Actuator{
protected:
    PwmChannelBase & instanceP;
    PwmChannelBase & instanceN;
    bool enabled = true;
public:
    Coil(PwmChannelBase & _instanceP, PwmChannelBase & _instanceN):instanceP(_instanceP), instanceN(_instanceN){;}

    void init() override{;}

    void setClamp(const real_t & abs_max_value){
        instanceP.setClamp(abs(abs_max_value));
        instanceN.setClamp(abs(abs_max_value));
    }

    void enable(const bool en = true) override{
        enabled = en;
        if(!en) setDuty(real_t(0));
    }

    void setDuty(const real_t & duty) override{
        if(!enabled){
            instanceP.setDuty(real_t(0));
            instanceN.setDuty(real_t(0));
            return;
        }
        if(duty > 0){
            instanceP.setDuty(duty);
            instanceN.setDuty(real_t(0));
        }else{
            instanceP.setDuty(real_t(0));
            instanceN.setDuty(-duty);
        }
    }
};

#endif