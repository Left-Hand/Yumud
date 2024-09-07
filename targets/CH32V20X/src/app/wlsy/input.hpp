#ifndef __INPUT_HPP__

#define __INPUT_HPP__

#include "wlsy_inc.hpp"
#include "power.hpp"

namespace WLSY{

using LowpassFilter = LowpassFilter_t<real_t, real_t>;
using Sys::t;
using Sys::Clock::reCalculateTime;

class InputModule:public WattMonitor{
protected:
    INA226 & ina;
    Heater & ht;
    NTC & ntc_l;
    NTC & ntc_h;

    real_t targ_watt = 0;
public:
    InputModule(INA226 & _ina, Heater & _ht, NTC & _ntc_l, NTC & _ntc_h):ina(_ina), ht(_ht), ntc_l(_ntc_l), ntc_h(_ntc_h){;}

    void setWatt(const real_t & watt){
        targ_watt = watt;
        ht.setTargetWatt(targ_watt);
    }

    auto getHighTemperature(){
        return ntc_h.getTemp();
    }

    auto getLowTemperature(){
        return ntc_l.getTemp();
    }

    real_t getTargWatt(){
        return targ_watt;
    }
    real_t getWatt() override{
        real_t watt = ina.getPower();
        return MAX(watt, 0);
    }

    real_t getVolt(){
        return ina.getVoltage();
    }

    real_t getAmps(){
        return MAX(ina.getCurrent(), 0);
    }

    void init(){
        ina.init(real_t(0.009), real_t(5.0));
        ht.init();
    }

    void run(){
        ina.update();
        ht.run();
    }
};
}

#endif