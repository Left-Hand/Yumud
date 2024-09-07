#ifndef __BACK_END_HPP__

#define __BACK_END_HPP__

#include "wlsy_inc.hpp"
#include "input.hpp"
#include "output.hpp"

namespace WLSY{

class BackModule{
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
    }

    auto getInputModuleInfos(){
        struct{
            real_t volt;
            real_t amps;
            real_t watt;
        }info{
            .volt = im.getVolt(),
            .amps = im.getAmps(),
            .watt = im.getWatt()
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

    real_t getInputWatt(){
        return im.getWatt();
    }

    real_t getOutputWatt(){
        return om.getWatt();
    }

    real_t getEffiency(){
        return om.getWatt() / im.getWatt();
    }
};


}

#endif