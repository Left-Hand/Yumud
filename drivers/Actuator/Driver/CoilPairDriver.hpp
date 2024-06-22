#ifndef __COIL_PAIR_DRIVER_HPP__

#define __COIL_PAIR_DRIVER_HPP__

#include "CoilDriver.hpp"

class CoilPairDriver:public CoilDriver{
    virtual void setDuty(const real_t & duty) = 0;
}

class CoilPairDriverDualPwm:public CoilDriverCurrentOpenloop, CoilPairDriver{
protected:
    PwmChannel & m_chp;
    PwmChannel & m_chn;
public:
    CoilPairDriverDualPwm(PwmChannel & chp, PwmChannel & chn):m_chp(chp),m_chn(chn){;}
    void setDuty(const real_t & duty) override{
        if(!enabled)return;
        if(duty > 0){
            m_chp.setDuty(duty);
            m_chn.setDuty(real_t(0));
        }else{
            m_chp.setDuty(real_t(0));
            m_chn.setDuty(-duty);
        }
    };

    void init() override{
        m_chp.init();
        m_chn.init();
    }

    void enable(const bool & en = true) override{
        enabled = en;
        if(!en){
            m_chp.setDuty(real_t(0));
            m_chn.setDuty(real_t(0));
        }
    }
};

class CoilPairDriverVref:public CoilDriverCurrentOpenloop, CoilPairDriver{
protected:
    GpioConcept & m_gp;
    GpioConcept & m_gn;


    PwmChannel & m_vref;
public:
    CoilPairDriverVref(const real_t & c2d_scale, GpioConcept & gp, GpioConcept & gn, PwmChannel & vref):CoilDriverCurrentOpenloop(c2d_scale), m_gp(gp),m_gn(gn),m_vref(vref){;}

    void setDuty(const real_t & duty) override{
        if(!enabled)return;
        if(duty > 0){
            m_gp.set();
            m_gn.clr();
        }else{
            m_gp.clr();
            m_gn.set();
        }

        m_vref.setDuty(abs(duty));
    }

    void init() override{
        m_gp.outpp();
        m_gn.outpp();
        m_vref.init();
    }

    void enable(const bool & en = true) override{
        enabled = en;
        if(!en){
            m_gp.clr();
            m_gn.clr();
            m_vref = real_t(0);
        }
    }
};

class CoilPairDriverMultiple: public CoilDriverCurrentOpenloop, CoilPairDriver{
protected:
    PwmChannel & m_chp;
    PwmChannel & m_chn;
    PwmChannel & m_vref;

    real_t m_soft_threshold = real_t(0.5);
public:
    CoilPairDriverMultiple(const real_t & c2d_scale, GpioConcept & chp, GpioConcept & chn, PwmChannel & vref, const real_t & soft_threshold):
        CoilDriverCurrentOpenloop(), m_chp(chp), m_chn(chn), m_vref(vref), m_soft_threshold(soft_threshold){;}

    void setDuty(const real_t & duty) override{
        if(!enabled)return;
        if(abs(duty) > m_soft_threshold){
            if(duty > 0){
                m_chp.setDuty(duty);
                m_chn.setDuty(real_t(0));
            }else{
                m_chp.setDuty(real_t(0));
                m_chn.setDuty(-duty);
            }
            m_vref.setDuty(real_t(1));
        }else{
            if(duty > 0){
                m_chp = real_t(1);
                m_chn = real_t(0);
            }else{
                m_chp = real_t(0);
                m_chn = real_t(1);
            }

            m_vref.setDuty(abs(duty));
        }

    }

    void init() override{
        m_chp = real_t(0);
        m_chn = real_t(0);
        m_vref = real_t(0);
        m_chp.init();
        m_chn.init();
        m_vref.init();
    }

};
#endif