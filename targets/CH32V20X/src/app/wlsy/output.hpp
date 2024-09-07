#ifndef __OUTPUT_HPP__

#define __OUTPUT_HPP__

#include "wlsy_inc.hpp"
// #include ""
namespace WLSY{

class SpeedCapture{
// protected:
public:
    LowpassFilter lpf{real_t(10.0)};
    CaptureChannelExti & cap;
    real_t dur;//ms
    
    static constexpr uint8_t poles = 9;
    static constexpr real_t rad_delta = real_t(TAU / poles);
    static constexpr real_t radius = real_t(0.05);
public:
    real_t rad;
    real_t omega;
    real_t speed;

    SpeedCapture(CaptureChannelExti & cap):cap(cap){};

    void init(){
        dur = 0;
        rad = 0;
        omega = 0;
        speed = 0;

        cap.init();
        cap.bindCb([this](){this->update();});
    }

    void update(){
        dur = lpf.update(real_t(cap.getPeriodUs()) / 1000, t);
        rad += rad_delta;
        omega = rad_delta / (dur / 1000);
        speed = omega * radius;
    }

    real_t getSpeed(){
        return speed;
    }
};

class OutputModule:public WattMonitor{
public:
    SpeedCapture & sc;
    HX711 & hx;
public:
    OutputModule(SpeedCapture & _sc, HX711 & _hx):sc(_sc), hx(_hx){;}
    real_t getWatt() override{
        return MAX(getSpeed() * getForce(), 0);
    }

    real_t getSpeed(){
        return sc.getSpeed();
    }

    real_t getForce(){
        return MAX(hx.getNewton(), 0) * 4;
    }

    void init(){
        sc.init();
        hx.init();
        hx.setConvType(HX711::ConvType::A128);
        hx.compensate();
    }

    void run(){
        hx.update();
    }
};



}
#endif