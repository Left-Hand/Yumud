#ifndef __COIL_HPP__
#define __COIL_HPP__

#include "src/device/Actuator/Actuator.hpp"
#include "src/gpio/gpio.hpp"

class CoilConcept: public Actuator{

};

class Coil1:public CoilConcept{
protected:
    GpioConcept & gpioP;
    GpioConcept & gpioN;
    PwmChannelConcept & instance;
    bool enabled = true;
public:
    Coil1(GpioConcept & _instanceP, GpioConcept & _instanceN, PwmChannelConcept & _instance):gpioP(_instanceP), gpioN(_instanceN), instance(_instance){;}

    void init() override{
        gpioP.OutPP();
        gpioN.OutPP();
        instance.init();
    }

    void setClamp(const real_t & abs_max_value){
        instance.setClamp(abs(abs_max_value));
    }

    void enable(const bool en = true) override{
        enabled = en;
        if(!en) setDuty(real_t(0));
    }

    void setDuty(const real_t & duty) override{
        if(!enabled){
            gpioN.clr();
            gpioP.clr();
            return;
        }
        if(duty > 0){
            gpioP.set();
            gpioN.clr();
            instance.setDuty(duty);
        }else{
            gpioP.clr();
            gpioN.set();
            instance.setDuty(-duty);
        }
    }

    Coil1 & operator = (const real_t & duty) override {setDuty(duty); return *this;}
};


class Coil2:public CoilConcept{
protected:
    PwmChannelConcept & instanceP;
    PwmChannelConcept & instanceN;
    bool enabled = true;
public:
    Coil2(PwmChannelConcept & _instanceP, PwmChannelConcept & _instanceN):instanceP(_instanceP), instanceN(_instanceN){;}

    void init() override{
        instanceP.init();
        instanceN.init();
    }

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