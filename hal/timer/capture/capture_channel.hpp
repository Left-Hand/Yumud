#pragma once

#include "core/clock/clock.hpp"
#include "core/clock/time_stamp.hpp"
#include "hal/exti/exti.hpp"
#include "core/math/real.hpp"
#include "hal/gpio/gpio.hpp"

namespace ymd::hal{

class CaptureChannelIntf{
public:
    virtual Microseconds get_pulse_us() const = 0;
    virtual Microseconds get_period_us() const = 0;
};

struct CaptureChannelUtils{ 

    template<typename T, typename U>
    static constexpr auto map_pulse_and_period_to_duty(
        const T pulse_, 
        const U period_
    ) {
        if constexpr(is_fixed_point_v<real_t>){
            return pulse_ / period_;
        }else{
            return pulse_ / period_;
        }
    }

    template<typename T, typename U>
    static constexpr auto map_unit_and_period_to_freq(
        const T unit_, 
        const U period_
    ){
        if constexpr(is_fixed_point_v<real_t>){
            return unit_ / period_;
        }else{
            return unit_ / period_;
        }
    }
};


class CaptureChannelExti final:public CaptureChannelIntf{
public:
    void update(){
        if(double_edge_){
            if(instance_.p_gpio_ == nullptr) return;
            bool val = instance_.p_gpio_->read() == HIGH;

            if(val == false){
                const auto current_t = clock::micros();
                pulse_ = current_t - last_t_;
                last_t_ = current_t;
            }else{
                const auto current_t = clock::micros();
                period_ = current_t - last_t_ + pulse_;
                last_t_ = current_t;
                EXECUTE(cb_);
            }
        }else{
            const auto current_t = clock::micros();
            period_ = current_t - last_t_;
            last_t_ = current_t;
            EXECUTE(cb_);
        }
    }
public:
    CaptureChannelExti(ExtiChannel & instance):
        instance_(instance),
        unit_(1000000), 
        double_edge_(instance.edge_ == ExtiTrigEdge::Dual){;}
    void init(){
        instance_.init();
        instance_.bind_cb([this](){this->update();});
        instance_.enable_it(EN);
    }

    template<typename Fn>
    void bind_cb(Fn && _cb){
        cb_ = std::function<void(void)>(std::move(_cb));
    }

    Microseconds get_pulse_us() const{
        return pulse_;
    }

    Microseconds get_period_us() const{
        return period_;
    }
private:
    ExtiChannel & instance_;
    const uint32_t unit_;
    const bool double_edge_;
    std::function<void(void)> cb_ = nullptr;
    Microseconds last_t_ = 0us;
    Microseconds pulse_ = 0us;
    Microseconds period_ = 0us;
};


}