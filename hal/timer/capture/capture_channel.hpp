#pragma once

#include "core/clock/clock.hpp"
#include "core/clock/time_stamp.hpp"
#include "hal/exti/exti.hpp"
#include "core/math/real.hpp"
#include "hal/gpio/gpio.hpp"

namespace ymd::hal{

class CaptureChannelConcept{
protected:
    uint32_t pulse = 0;
    uint32_t period = 0;
    const uint32_t unit;
    const bool double_edge;
public:
    CaptureChannelConcept(const uint32_t _unit,const bool _double_edge): unit(_unit), double_edge(_double_edge){;}
    real_t getFreq() const{
        // #ifdef USE_IQ
        if constexpr(is_fixed_point_v<real_t>){
            // real_t unit_value;
            // real_t period_value;
            // unit_value.value = _iq<16>(unit);
            // period_value.value = _iq<16>(period);
            return unit / period;
        }else{
            return unit / period;
        }
        //     float unit_value = unit;
        //     float period_value = period;
        //     return unit_value / period_value;
        // }
    }

    real_t getDuty() const{
        if constexpr(is_fixed_point_v<real_t>){
            // real_t pulse_value;
            // real_t period_value;
            // pulse_value.value = _iq<16>(pulse);
            // period_value.value = _iq<16>(period);
            // return pulse_value/period_value;
            return unit / period;

        }else{
            return pulse/period;
        }
    }

    virtual uint32_t getPulseUs() const = 0;
    virtual uint32_t getPeriodUs() const = 0;
    virtual void init() = 0;
};

class CaptureChannel:public CaptureChannelConcept{

};

class CaptureChannelExti:public CaptureChannelConcept{
protected:
    ExtiChannel instance;
    uint32_t last_t;
    std::function<void(void)> cb;

    void update(){
        if(double_edge){
            if(instance.gpio == nullptr) return;
            bool val = instance.gpio->read();

            if(val == false){
                uint32_t current_t = micros();
                pulse = current_t - last_t;
                last_t = current_t;
            }else{
                uint32_t current_t = micros();
                period = current_t - last_t + pulse;
                last_t = current_t;
                EXECUTE(cb);
            }
        }else{
            uint32_t current_t = micros();
            period = current_t - last_t;
            last_t = current_t;
            EXECUTE(cb);
        }
    }
public:
    CaptureChannelExti(const ExtiChannel && _instance):
        CaptureChannelConcept(1000000, 
                _instance.trigger == ExtiChannel::Trigger::Dual), 
        instance(_instance){;}

    CaptureChannelExti(const ExtiChannel & _instance):CaptureChannelExti(std::move(_instance)){;}

    void init() override{
        instance.init();
        instance.bindCb([this](){this->update();});
        instance.enableIt();
    }

    void bindCb(std::function<void(void)> && _cb){
        cb = _cb;
    }

    uint32_t getPulseUs() const override{
        return pulse;
    }

    uint32_t getPeriodUs() const override{
        return period;
    }
};


}