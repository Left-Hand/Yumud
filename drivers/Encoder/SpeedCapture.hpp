#pragma once

#include "Encoder.hpp"


namespace yumud::drivers{
class SpeedCapture{
// protected:
public:
    CaptureChannelExti & cap;
    real_t dur_ms;//ms
    
    scexpr uint poles = 9;
    scexpr real_t rad_delta = real_t(TAU / poles);
    scexpr real_t radius = real_t(0.05);
public:
    real_t rad;
    real_t omega;
    real_t speed;

    uint32_t min_update_dur_ms = 160;
    uint32_t last_update_ms = 0;

    SpeedCapture(CaptureChannelExti & _cap):cap(_cap){};

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


}