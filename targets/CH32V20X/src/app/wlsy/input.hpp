#ifndef __INPUT_HPP__

#define __INPUT_HPP__

#include "wlsy_inc.hpp"

namespace WLSY{

using LowpassFilter = LowpassFilter_t<real_t, real_t>;
using Sys::t;
using Sys::Clock::reCalculateTime;
using Waveform = TJC::Waveform;
using WaveWindow = TJC::WaveWindow;


class WattMonitor{
    virtual real_t getWatt() = 0;
};

class InputModule:public WattMonitor{
protected:
    INA226 & ina;
    Heater & ht;
public:
    InputModule(INA226 & _ina, Heater & _ht):ina(_ina), ht(_ht){;}

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