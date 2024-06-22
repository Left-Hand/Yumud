#ifndef __COIL_HPP__
#define __COIL_HPP__

#include "drivers/Actuator/Actuator.hpp"
#include "hal/gpio/gpio.hpp"


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
    // PwmChannel & instance;
    PwmChannel & vref_pwm;
    bool enabled = true;
public:
    Coil1(GpioConcept & _instanceP, GpioConcept & _instanceN, PwmChannel & _vref_pwm):gpioP(_instanceP), gpioN(_instanceN), vref_pwm(_vref_pwm){;}

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
        if(!en) sleep();
    }

    void setDuty(const real_t & duty) override{
        // if(!enabled){
        //     gpioN.clr();
        //     gpioP.clr();
        //     return;
        // }

        // constexpr float curr_base = 0.02;
        vref_pwm = ABS(duty);

        switch (int(sign(duty))){
            case 1:
                gpioP.set();
                gpioN.clr();
                break;
            case 0:
                gpioP.set();
                gpioN.set();
                break;
            case -1:
                gpioP.clr();
                gpioN.set();
                break;
        }
    }

    void sleep(){
        gpioN.clr();
        gpioP.clr();
    }

    void brake(){
        gpioN.set();
        gpioP.set();
    }

    Coil1 & operator = (const real_t & duty) override {setDuty(duty); return *this;}
};

class TB67H450{
protected:
    TimerOC & forward_pwm;
    TimerOC & backward_pwm;
    PwmChannel & vref_pwm;
    bool enabled = true;
    bool hardmode = false;
    int full_current = 3;
public:
    TB67H450(TimerOC & _forward_pwm, TimerOC & _backward_pwm, PwmChannel & _vref_pwm):
            forward_pwm(_forward_pwm), backward_pwm(_backward_pwm), vref_pwm(_vref_pwm){;}

    void init(){

        // forward_pwm.enableSync();
        // backward_pwm.enableSync();
        // vref_pwm.enableSync();

        forward_pwm.setPolarity(false);
        backward_pwm.setPolarity(false);
        // vref_pwm.setPolarity(true);

        forward_pwm.init();
        backward_pwm.init();
        vref_pwm.init();

        setClamp(1.0);
    }

    void setClamp(const real_t abs_max_value){
        vref_pwm = ABS(abs_max_value);
    }

    void enable(const bool en = true){
        enabled = en;
        if(!en){
            forward_pwm = real_t(1);
            backward_pwm = real_t(1);
            vref_pwm = real_t(0);
        }
    }

    void setCurrent(const real_t curr){

        if(!enabled) return;

        if(hardmode){
            if(curr > 0){
                forward_pwm = 0;
                backward_pwm = curr * full_current;
            }else if(curr < 0){
                forward_pwm = -curr * full_current;
                backward_pwm = 0;
            }else{
                forward_pwm = 0;
                backward_pwm = 0;
            }
        }
        
        else{
            if(curr > 0){
                forward_pwm = real_t(0);
                backward_pwm = real_t(1);
                vref_pwm = curr / 3.3;
            }else{
                forward_pwm = real_t(1);
                backward_pwm = real_t(0);
                vref_pwm = -curr / 3.3;
            }
        }
    }

    TB67H450 & operator = (const real_t curr){setCurrent(curr); return *this;}
};
class Coil2:public Coil2PConcept{
protected:
    PwmChannel & instanceP;
    PwmChannel & instanceN;
    bool enabled = true;
public:
    Coil2(PwmChannel & _instanceP, PwmChannel & _instanceN):instanceP(_instanceP), instanceN(_instanceN){;}

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
            instanceP = 0;
            instanceN = 0;
            return;
        }
        if(duty > 0){
            instanceP = duty;
            instanceN = 0;
        }else{
            instanceP = 0;
            instanceN = -duty;
        }
    }

    Coil2 & operator = (const real_t & duty) override {setDuty(duty); return *this;}
};


// class Coil3:public Coil3PConcept{
// protected:
//     PwmChannel & instanceU;
//     PwmChannel & instanceV;
//     PwmChannel & instanceW;
//     bool enabled = true;
// public:
//     Coil3(PwmChannel & _instanceU, PwmChannel & _instanceV, PwmChannel & _instanceW):
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