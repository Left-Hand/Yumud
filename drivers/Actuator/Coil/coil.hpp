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

class TB67H450:public Coil2PConcept{
protected:
    PwmChannel & forward_pwm;
    PwmChannel & backward_pwm;
    PwmChannel & vref_pwm;
    bool enabled = true;
    bool hardmode = false;
public:
    TB67H450(PwmChannel & _forward_pwm, PwmChannel & _backward_pwm, PwmChannel & _vref_pwm):
            forward_pwm(_forward_pwm), backward_pwm(_backward_pwm), vref_pwm(_vref_pwm){;}

    void init() override{

        // forward_pwm.enableSync();
        // backward_pwm.enableSync();
        // vref_pwm.enableSync();

        // forward_pwm.setPolarity(f);
        // backward_pwm.setPolarity(false);
        // vref_pwm.setPolarity(true);

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