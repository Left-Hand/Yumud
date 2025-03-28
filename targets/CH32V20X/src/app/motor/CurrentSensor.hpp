#pragma once

#include "utils.hpp"
#include "concept/analog_channel.hpp"
#include "dsp/filter/butterworth/ButterSideFilter.hpp"

namespace ymd::foc{


class CurrentSensor{
protected:
    AnalogInIntf & u_sense_;
    AnalogInIntf & v_sense_;
    AnalogInIntf & w_sense_;

    UvwCurrent uvw_bias_;
    UvwCurrent uvw_raw_;
    UvwCurrent uvw_curr_;
    real_t mid_curr_;
    AbCurrent ab_curr_;
    DqCurrent dq_curr_;

    dsp::ButterLowpassFilter<q16, 2> mid_filter_ = {{
        .fc = 400,
        .fs = foc_freq
    }};

public:
    CurrentSensor(
        AnalogInIntf & u_sense,
        AnalogInIntf & v_sense, 
        AnalogInIntf & w_sense
    ): 
        u_sense_(u_sense),
        v_sense_(v_sense), 
        w_sense_(w_sense){
            reset();
        }

    void reset(){
        uvw_curr_ = {0, 0, 0};
        uvw_bias_ = {0, 0, 0};
        ab_curr_ = {0, 0};
        dq_curr_ = {0, 0};
    }


    void capture(){
        uvw_raw_ = {
            real_t(u_sense_),
            real_t(v_sense_),
            real_t(w_sense_)
        };

        mid_filter_.update((uvw_raw_.u + uvw_raw_.v + uvw_raw_.w) / 3);
        mid_curr_ = mid_filter_.get();
        uvw_curr_[0] = (uvw_raw_.u - mid_curr_ - uvw_bias_.u);
        uvw_curr_[1] = (uvw_raw_.v - mid_curr_ - uvw_bias_.v);
        uvw_curr_[2] = (uvw_raw_.w - mid_curr_ - uvw_bias_.w);

        ab_curr_ = uvw_to_ab(uvw_curr_);
    }


    void update(const real_t rad){

        capture();

        dq_curr_ = ab_to_dq(ab_curr_, rad);
        
        // if(likely(ABS(dq_curr[0]) < 1))dq_curr_[0] = LPFN<7>(dq_curr_[0],dq_curr[0]);
        // if(likely(ABS(dq_curr[1]) < 1))dq_curr_[1] = LPFN<7>(dq_curr_[1],dq_curr[1]);

        // dq_curr_[0] = LPFN<7>(dq_curr_[0],dq_curr[0]);
        // dq_curr_[1] = LPFN<7>(dq_curr_[1],dq_curr[1]);

    }
    auto raw()const {return uvw_raw_;}

    auto mid() const {return mid_curr_;}
    auto uvw()const{return uvw_curr_;}
    // auto uvw(){return uvw_curr_;}
    auto ab()const{return ab_curr_;}
    // auto ab(){return ab_curr_;}
    auto dq()const{return dq_curr_;}
    // auto dq(){return dq_curr_;}
};

}