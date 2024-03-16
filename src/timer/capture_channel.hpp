#ifndef __CAPTURE_CHANNEL_HPP__

#define __CAPTURE_CHANNEL_HPP__

#include "src/clock/clock.h"
#include "src/clock/time_stamp.hpp"
#include "src/exti/exti.hpp"
#include "real.hpp"

class CaptureChannelConcept{
protected:
    uint32_t pulse = 0;
    uint32_t period = 0;
    const uint32_t unit;
    const bool double_edge;
public:
    CaptureChannelConcept(const uint32_t & _unit,const bool & _double_edge): unit(_unit), double_edge(_double_edge){;}
    real_t getFreq() const{
        #ifdef USE_IQ
        iq_t unit_value;
        iq_t period_value;
        unit_value.value = unit;
        period_value.value = period;
        #else
        float unit_value = unit;
        float period_value = period;
        #endif

        return unit_value / period_value;
    }

    real_t getDuty() const{
        #ifdef USE_IQ
        iq_t pulse_value;
        iq_t period_value;
        pulse_value.value = pulse;
        period_value.value = period;
        #else
        float pulse_value = pulse;
        float period_value = period;
        #endif

        return pulse_value / period_value;
    }
};

class CaptureChannel:public CaptureChannelConcept{

};

class CaptureChannelExti:public CaptureChannelConcept{
protected:
    ExtiChannel & instance;
    uint32_t last_t;
    volatile bool state;

    void update(){
        if(double_edge){
            if(state){
                uint32_t current_t = micros();
                pulse = current_t - last_t;
                last_t = current_t;
                state = false;
            }else{
                uint32_t current_t = micros();
                period = current_t - last_t + pulse;
                last_t = current_t;
                state = true;
            }
        }else{
            uint32_t current_t = micros();
            period = current_t - last_t;
            last_t = current_t;
        }
    }
public:
    CaptureChannelExti(ExtiChannel & _instance, const bool & _double_edge):CaptureChannelConcept(1000000, _double_edge), instance(_instance){;}
    void init(){
        instance.init();
        instance.bindCb(std::bind(&CaptureChannelExti::update, this));
        instance.enableIt();
    }

};
#endif