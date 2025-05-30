#pragma once

#include "Encoder.hpp"


namespace ymd::drivers{
class SpeedCapture{
// protected:
public:
    CaptureChannelExti & cap;
    real_t dur_ms;//ms
    
    static constexpr size_t poles = 9;
    static constexpr real_t rad_delta = real_t(TAU / poles);
    static constexpr real_t radius = real_t(0.05);
public:
    real_t rad;
    real_t omega;
    real_t speed;

    Milliseconds MIN_UPDATE_DUR_MS = 160ms;
    Milliseconds last_update_ms_ = 0ms;

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
        last_update_ms_ = millis();
        if(millis() - last_update_ms_ >MIN_UPDATE_DUR_MS) speed = 0;
    }

    real_t get_speed(){
        return speed;
    }
};


}