#ifndef __OUTPUT_HPP__

#define __OUTPUT_HPP__

#include "wlsy_inc.hpp"
#include "dsp/filter/BurrFilter.hpp"
// #include ""
namespace WLSY{

using BurrFilter = BurrFilter_t<real_t>;
class SpeedCapture{
// protected:
public:
    CaptureChannelExti & cap;
    real_t dur_ms;//ms
    
    static constexpr uint8_t poles = 9;
    static constexpr real_t rad_delta = real_t(TAU / poles);
    static constexpr real_t radius = real_t(0.05);
public:
    real_t rad;
    real_t omega;
    real_t speed;

    uint32_t min_update_dur_ms = 160;
    uint32_t last_update_ms = 0;

    SpeedCapture(CaptureChannelExti & cap):cap(cap){};

    void init(){
        dur_ms = 0;
        rad = 0;
        omega = 0;
        speed = 0;

        cap.init();
        cap.bindCb([this](){this->update();});
    }

    void update(){
        dur_ms = real_t(cap.getPeriodUs() / 100) / 10;
        rad += rad_delta;
        omega = 1000 * rad_delta / dur_ms;
        speed = omega * radius;
        last_update_ms = millis();
    }

    void run(){
        if(millis() - last_update_ms >min_update_dur_ms) speed = 0;
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

    real_t getRawSpeed(){
        return sc.getSpeed();
    }

    real_t getSpeed(){
        static LowpassFilter lpf{real_t(0.4)};
        static LowpassFilter lpf2{real_t(0.4)};

        return lpf2.update(lpf.update(getRawSpeed(), t), t);
    }

    real_t getForce(){
        // static auto bf = BurrFilter{real_t0.5, 0.15, 10};
        static LowpassFilter lpf{real_t(0.4)};
        return lpf.update((MAX(hx.getNewton(), 0) * 4), t);
        // return bf.update(MAX(hx.getNewton(), 0) * 4);
        // return hx.getNewton() * 4;

    }

    void init(){
        sc.init();
        hx.init();
        hx.setConvType(HX711::ConvType::A128);
        hx.compensate();
    }

    void run(){
        sc.run();
        hx.update();
    }
};



}
#endif