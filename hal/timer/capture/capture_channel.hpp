#ifndef __CAPTURE_CHANNEL_HPP__

#define __CAPTURE_CHANNEL_HPP__

#include "../sys/kernel/clock.h"
#include "../sys/kernel/time_stamp.hpp"
#include "../hal/exti/exti.hpp"
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
        unit_value.value = _iq(unit);
        period_value.value = _iq(period);
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
        pulse_value.value = _iq(pulse);
        period_value.value = _iq(period);
        iq_t ret = pulse_value/period_value;
        // iq_t ret = pulse_value/period_value;
        // if((pulse > (period - pulse)) ^ (ret.value < 0.5)) ret = 1 - ret;
        #else
        float pulse_value = pulse;
        float period_value = period;
        float ret = pulse_value/period_value;
        #endif

        return ret;
    }

    virtual uint32_t getPulseUs() const = 0;
    virtual uint32_t getPeriodUs() const = 0;
    virtual void init() = 0;
};

class CaptureChannel:public CaptureChannelConcept{

};

class CaptureChannelExti:public CaptureChannelConcept{
protected:
    ExtiChannel & instance;
    Gpio & gpio;
    uint32_t last_t;
    std::function<void(void)> cb;

    void update(){
        if(double_edge){
            if(!gpio.read()){
                uint32_t current_t = micros();
                pulse = current_t - last_t;
                last_t = current_t;
            }else{
                uint32_t current_t = micros();
                period = current_t - last_t + pulse;
                last_t = current_t;
                if(cb) cb();
            }
        }else{
            uint32_t current_t = micros();
            period = current_t - last_t;
            last_t = current_t;
            if(cb) cb();
        }
    }
public:
    CaptureChannelExti(ExtiChannel & _instance, Gpio & _gpio):CaptureChannelConcept(1000000, _instance.trigger == ExtiChannel::Trigger::RisingFalling), instance(_instance), gpio(_gpio){;}

    void init() override{
        instance.setPinMode(PinMode::InPullDN);
        instance.init();
        instance.bindCb([this](){this->update();});
        instance.enableIt();
    }

    void bindCb(const std::function<void(void)>& _cb){
        cb = _cb;
    }

    uint32_t getPulseUs() const override{
        return pulse;
    }

    uint32_t getPeriodUs() const override{
        return period;
    }
};
#endif