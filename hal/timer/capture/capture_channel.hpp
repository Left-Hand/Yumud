#pragma once

#include "core/clock/clock.hpp"
#include "core/clock/time_stamp.hpp"
#include "hal/exti/exti.hpp"
#include "core/math/real.hpp"
#include "hal/gpio/gpio.hpp"

namespace ymd::hal{

class CaptureChannelIntf{
    virtual Microseconds getPulseUs() const = 0;
    virtual Microseconds getPeriodUs() const = 0;
    virtual void init() = 0;
};

class CaptureChannelConcept:public CaptureChannelIntf{
protected:
    Microseconds pulse = 0us;
    Microseconds period = 0us;
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
            return unit / period.count();
        }else{
            return unit / period.count();
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
            return unit / period.count();

        }else{
            return pulse.count() / period.count();
        }
    }
};

class CaptureChannel:public CaptureChannelConcept{

};

class CaptureChannelExti:public CaptureChannelConcept{
protected:
    ExtiChannel instance;
    Microseconds last_t;
    std::function<void(void)> cb;

    void update(){
        if(double_edge){
            if(instance.gpio == nullptr) return;
            bool val = bool(instance.gpio->read());

            if(val == false){
                const auto current_t = clock::micros();
                pulse = current_t - last_t;
                last_t = current_t;
            }else{
                const auto current_t = clock::micros();
                period = current_t - last_t + pulse;
                last_t = current_t;
                EXECUTE(cb);
            }
        }else{
            const auto current_t = clock::micros();
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

    Microseconds getPulseUs() const override{
        return pulse;
    }

    Microseconds getPeriodUs() const override{
        return period;
    }
};


}