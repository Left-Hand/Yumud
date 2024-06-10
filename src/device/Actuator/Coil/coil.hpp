#ifndef __COIL_HPP__
#define __COIL_HPP__

#include "src/device/Actuator/Actuator.hpp"
#include "src/gpio/gpio.hpp"


class CoilConcept: public Actuator{
public:
    virtual void setClamp(const real_t & abs_max_value) = 0;
    virtual void setDuty(const real_t & duty) = 0;
};
class Coil2PConcept: public CoilConcept{
public:
    virtual Coil2PConcept& operator= (const real_t & duty) = 0;
};



class Coil1:public Coil2PConcept{
protected:
    GpioConcept & gpioP;
    GpioConcept & gpioN;
    // PwmChannelConcept & instance;
    TimerOutChannelPosOnChip & vref_pwm;
    bool enabled = true;
public:
    Coil1(GpioConcept & _instanceP, GpioConcept & _instanceN, TimerOutChannelPosOnChip & _vref_pwm):gpioP(_instanceP), gpioN(_instanceN), vref_pwm(_vref_pwm){;}

    void init() override{
        gpioP.OutPP();
        gpioN.OutPP();

        gpioP.clr();
        gpioN.clr();

        vref_pwm.init();
    }

    void setClamp(const real_t & abs_max_value) override{
        // instance.setClamp(abs(abs_max_value));
    }

    void enable(const bool & en = true) override{
        enabled = en;
        if(!en) setDuty(real_t(0));
    }

    void setDuty(const real_t & duty) override{
        if(!enabled){
            gpioN.clr();
            gpioP.clr();
            return;
        }

        // constexpr float curr_base = 0.02;
        if(duty > 0){
            gpioP.set();
            gpioN.clr();
            vref_pwm = duty;
        }else{
            gpioN.set();
            gpioP.clr();
            vref_pwm = -duty;
        }
    }

    Coil1 & operator = (const real_t & duty) override {setDuty(duty); return *this;}
};

class TB67H450:public Coil2PConcept{
protected:
    TimerOutChannelPosOnChip & forward_pwm;
    TimerOutChannelPosOnChip & backward_pwm;
    TimerOutChannelPosOnChip & vref_pwm;
    bool enabled = true;
    bool hardmode = false;
public:
    TB67H450(TimerOutChannelPosOnChip & _forward_pwm, TimerOutChannelPosOnChip & _backward_pwm, TimerOutChannelPosOnChip & _vref_pwm):
            forward_pwm(_forward_pwm), backward_pwm(_backward_pwm), vref_pwm(_vref_pwm){;}

    void init() override{

        forward_pwm.enableSync();
        backward_pwm.enableSync();
        vref_pwm.enableSync();

        forward_pwm.setPolarity(false);
        backward_pwm.setPolarity(false);
        vref_pwm.setPolarity(true);

        forward_pwm.init();
        backward_pwm.init();
        vref_pwm.init();
    }

    void setClamp(const real_t & abs_max_value) override{
        vref_pwm = abs(abs_max_value);
    }

    void enable(const bool & en = true) override{
        enabled = en;
        if(!en){
            forward_pwm = real_t(1);
            backward_pwm = real_t(1);
            vref_pwm = real_t(0);
        }
    }

    void setDuty(const real_t & duty) override{

        if(!enabled) return;

        if(hardmode){
            if(duty > 0){
                forward_pwm = 0;
                backward_pwm = duty;
            }else{
                forward_pwm = -duty;
                backward_pwm = 0;
            }
        }else{
            if(duty > 0){
                forward_pwm = real_t(0);
                backward_pwm = real_t(1);
                vref_pwm = duty;
            }else{
                forward_pwm = real_t(1);
                backward_pwm = real_t(0);
                vref_pwm = -duty;
            }
        }
    }

    TB67H450 & operator = (const real_t & duty) override {setDuty(duty); return *this;}
};
class Coil2:public Coil2PConcept{
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

    void setClamp(const real_t & max_value) override{
        real_t abs_max_value = abs(max_value);
        instanceP.setClamp(abs_max_value);
        instanceN.setClamp(abs_max_value);
    }

    void enable(const bool & en = true) override{
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

    Coil2 & operator = (const real_t & duty) override {setDuty(duty); return *this;}
};


// class Coil3:public Coil3PConcept{
// protected:
//     PwmChannelConcept & instanceU;
//     PwmChannelConcept & instanceV;
//     PwmChannelConcept & instanceW;
//     bool enabled = true;
// public:
//     Coil3(PwmChannelConcept & _instanceU, PwmChannelConcept & _instanceV, PwmChannelConcept & _instanceW):
//             instanceU(_instanceU), instanceV(_instanceV), instanceW(_instanceW){;}

//     void init() override{
//         instanceU.init();
//         instanceV.init();
//         instanceW.init();
//     }

//     void setClamp(const real_t & max_value){
//         real_t abs_max_value = abs(max_value);
//         instanceU.setClamp(abs_max_value);
//         instanceV.setClamp(abs_max_value);
//         instanceW.setClamp(abs_max_value);
//     }
// };
#endif