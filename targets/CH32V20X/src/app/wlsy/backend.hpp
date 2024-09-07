#ifndef __BACK_END_HPP__

#define __BACK_END_HPP__

#include "wlsy_inc.hpp"
#include "input.hpp"
#include "output.hpp"
#include "integrator.hpp"

namespace WLSY{

class BackModule{
protected:
    Intergrator ipower_intergrator;
    Intergrator opower_intergrator;

    uint32_t last_set_ms = 0;
public:
    InputModule & im;
    OutputModule & om;
public:
    BackModule(InputModule & _im, OutputModule & _om):im(_im), om(_om){;}

    void init(){
        im.init();
        om.init();
    }

    void run(){
        im.run();
        om.run();

        ipower_intergrator.update(im.getWatt());
        opower_intergrator.update(om.getWatt());
    }

    auto getInputModuleInfos(){
        struct{
            real_t volt;
            real_t amps;
            real_t watt;
            real_t tmp_l;
            real_t tmp_h;
        }info{
            .volt = im.getVolt() > 4 ? im.getVolt() : 0,
            .amps = im.getAmps(),
            .watt = im.getWatt(),
            .tmp_l = im.getLowTemperature(),
            .tmp_h = im.getHighTemperature()
        };

        return info;
    }

    auto getOutputModuleInfos(){
        struct{
            real_t speed;
            real_t force;
            real_t watt;
        }info{
            .speed = om.getSpeed(),
            .force = om.getForce(),
            .watt = om.getWatt()
        };

        return info;
    }

    void setInputWatt(const real_t & watt){
        im.setWatt(watt);
        last_set_ms = millis();
    }

    auto getLastSetMs(){
        return last_set_ms;
    }

    void peel(){
        om.hx.compensate();
    }

    void startIntergrator(){
        ipower_intergrator.start();
        opower_intergrator.start();
    }

    auto getAverage(){
        struct{
            real_t spower;
            real_t ipower;
            real_t opower;
            real_t effiency;
        }ret{
            .spower = im.getTargWatt(),
            .ipower = ipower_intergrator.getAverage(),
            .opower = opower_intergrator.getAverage(),
            .effiency = opower_intergrator.getAverage() / ipower_intergrator.getAverage()
        };

        return ret;
    }

    void stopIntergrator(){
        ipower_intergrator.stop();
        opower_intergrator.stop();
    }

    void start(){
        startIntergrator();
        // im.setWatt
    }

    real_t getInputWatt(){
        return im.getWatt();
    }

    real_t getOutputWatt(){
        return om.getWatt();
    }

    real_t getEffiency(){
        auto iw = im.getWatt();
        if(iw == 0) return 0;
        return om.getWatt() / iw;
    }
};


}

#endif