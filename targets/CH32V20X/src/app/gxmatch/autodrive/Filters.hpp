#pragma once

#include "drivers/IMU/IMU.hpp"
#include "drivers/Proximeter/FlowSensor.hpp"

namespace gxm{

using namespace ymd::drivers;
// using Accerometer = ymd::Accerometer;

struct TauWrapper{
protected:
    real_t last_value_;
    real_t accumulated_value_;

    bool inited = false;
public:
    TauWrapper(){;}

    real_t update(const real_t value){
        if(!inited){
            last_value_ = value;
            accumulated_value_ = 0;
            inited = true;
        }else{
            auto delta = value - last_value_;

            scexpr auto one = real_t(PI);
            scexpr auto half_one = real_t(PI/2);

            if(delta > half_one){
                delta -= one;
            }else if (delta < -half_one){
                delta += one;
            }

            last_value_ = value;
            accumulated_value_ += delta;
        }


        return accumulated_value_;
    }

    void reset(){
        last_value_ = 0;
        accumulated_value_ = 0;
        inited = false;
    }
};



class ComplementaryFilter{
public:
    struct Config{
        real_t kq;
        real_t ko;
    };
    
protected:
    real_t rot_;
    real_t rot_unfiltered;
    real_t last_rot;
    real_t last_gyr;
    real_t last_time;
    
    bool inited;
    const Config & config_;
public:
    ComplementaryFilter(const Config & config):
        config_(config){
            reset();
        }

    real_t update(const real_t rot, const real_t gyr, const real_t time){

        do{
            if(!inited){
                rot_ = rot;
                rot_unfiltered = rot;
                inited = true;
            }else{
                const real_t delta_t = (time - last_time);
                rot_unfiltered += gyr * delta_t;
                rot_unfiltered = config_.kq * rot_ + (1-config_.kq) * rot;
                rot_ = config_.ko * rot_ + (1-config_.ko) * rot_unfiltered;
            }
        }while(false);
        
        last_rot = rot;
        last_gyr = gyr;
        last_time = time;
        return {rot_};
    }

    void reset(const real_t time = 0){
        rot_ = 0;
        rot_unfiltered = 0;
        last_rot = 0;
        last_gyr = 0;
        last_time = time;
        inited = false;
    }
};


class RotationObserver{
public:
    using Config = ComplementaryFilter::Config;
protected:
    TauWrapper tau_wrapper_;
    ComplementaryFilter comp_filter_;
public:
    RotationObserver(const Config & config):
        comp_filter_(config){
            reset();
        }

    real_t update(const real_t rot, const real_t gyr, const real_t time){
        return comp_filter_.update(
            tau_wrapper_.update(rot)
            , gyr, time);
    }

    void reset(const real_t time = 0){
        tau_wrapper_.reset();
        comp_filter_.reset();
    }
};

__fast_inline constexpr real_t min_rad_diff(real_t from, real_t to){
    from = fposmodp(from, real_t(TAU));
    to = fposmodp(to, real_t(TAU));

    const auto diff = to - from;
    return diff > real_t(PI) ? (diff - real_t(TAU)) : diff;
}


template<typename T>
class LowPassFilter_t{
public:
    struct Config{
        real_t kf;
    };
    
protected:
    T last_value_;
    // real_t last_time_;
    
    bool inited;
    const Config & config_;
public:
    LowPassFilter_t(const Config & config):
        config_(config){
            reset();
        }

    T update(const T value){

        do{
            if(!inited){
                last_value_ = value;
                inited = true;
            }else{
                last_value_ = LERP(last_value_, value, config_.kf);
            }
        }while(false);
        
        return {last_value_};
    }

    void reset(const real_t time = 0){
        last_value_ = T();
        // last_time_ = time;
        inited = false;
    }
};

}