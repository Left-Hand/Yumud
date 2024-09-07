#ifndef __SCENCES_HPP__

#define __SCENCES_HPP__

#include "wlsy_inc.hpp"
#include "backend.hpp"
#include "tjc.hpp"


namespace WLSY{
class Scene{
public:
    virtual void run();
};

class MainScene:public Scene{
protected:
public:
    void run() override{

    }
};

class PowerInScene:public Scene{
protected:
    BackModule & bm;
    Waveform & amps_wave;
    Waveform & volt_wave;
    Waveform & power_wave;
public:
    PowerInScene(BackModule & _bm, Waveform & _amps_wave, Waveform & _volt_wave, Waveform & _power_wave):
            bm(_bm), amps_wave(_amps_wave), volt_wave(_volt_wave), power_wave(_power_wave){;}
    void run() override{
        // auto im_info = bm.getInputModuleInfos();
        // amps_wave.addData(im_info.amps);
        // volt_wave.addData(im_info.volt);
        // power_wave.addData(im_info.watt);
    }
};

class PowerOutScene:public Scene{
protected:
    BackModule & bm;
    Waveform & speed_wave;
    Waveform & force_wave;
    Waveform & power_wave;
public:
    PowerOutScene(BackModule & _bm, Waveform & _speed_wave, Waveform & _force_wave, Waveform & _power_wave):
            bm(_bm), speed_wave(_speed_wave), force_wave(_force_wave), power_wave(_power_wave){;}
    void run() override{
        // auto om_info = bm.getOutputModuleInfos();
        // speed_wave.addData(om_info.speed);
        // force_wave.addData(om_info.force);
        // power_wave.addData(om_info.watt);
    }
};
}


#endif