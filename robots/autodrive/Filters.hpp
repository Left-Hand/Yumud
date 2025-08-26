#pragma once

#include "drivers/IMU/IMU.hpp"
#include "drivers/Proximeter/FlowSensor.hpp"
#include "dsp/filter/homebrew/ComplementaryFilter.hpp"

namespace ymd::robots{

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

            static constexpr auto one = real_t(PI);
            static constexpr auto half_one = real_t(PI/2);

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




class RotationObserver{
public:
    using ComplementaryFilter = ymd::dsp::ComplementaryFilter_t<real_t>;
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
        comp_filter_.update(
            tau_wrapper_.update(rot), 
            gyr
        );

        return comp_filter_.result();
    }

    void reset(const real_t time = 0){
        tau_wrapper_.reset();
        comp_filter_.reset();
    }
};

__fast_inline constexpr real_t min_rad_diff(real_t from, real_t to){
    from = fposmod(from, real_t(TAU));
    to = fposmod(to, real_t(TAU));

    const auto diff = to - from;
    return diff > real_t(PI) ? (diff - real_t(TAU)) : diff;
}


template<typename T>
class LowpassFilter{
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
    LowpassFilter(const Config & config):
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