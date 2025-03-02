#pragma once

#include "utils.hpp"
#include "hal/adc/analog_channel.hpp"

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

    template<size_t N>
    static __fast_inline iq_t<16> LPFN(const iq_t<16> x, const iq_t<16> y){
        constexpr size_t sc = ((1 << N) - 1);
        return (x * sc + y) >> N;
    }

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
        uvw_raw_ = {real_t(u_sense_),
                    real_t(v_sense_),
                    real_t(w_sense_)};
        mid_curr_ = LPFN<5>(mid_curr_, (uvw_raw_.u + uvw_raw_.v + uvw_raw_.w) / 3);
        uvw_curr_[0] = (uvw_raw_.u - mid_curr_ - uvw_bias_.u);
        uvw_curr_[1] = (uvw_raw_.v - mid_curr_ - uvw_bias_.v);
        uvw_curr_[2] = (uvw_raw_.w - mid_curr_ - uvw_bias_.w);
    }


    void update(const real_t rad){

        const real_t raw_u_curr = real_t(u_sense_);
        const real_t raw_v_curr = real_t(v_sense_);
        const real_t raw_w_curr = real_t(w_sense_);
        const real_t raw_sum_curr = raw_u_curr + raw_v_curr + raw_w_curr;
        mid_curr_ = LPFN<5>(mid_curr_, raw_sum_curr / 3);
        uvw_curr_[0] = (raw_u_curr - mid_curr_);
        uvw_curr_[1] = (raw_v_curr - mid_curr_);
        uvw_curr_[2] = (raw_w_curr - mid_curr_);

        ab_curr_ = uvw_to_ab(uvw_curr_);
        const auto dq_curr = ab_to_dq(ab_curr_, rad);
        
        // if(likely(ABS(dq_curr[0]) < 1))dq_curr_[0] = LPFN<7>(dq_curr_[0],dq_curr[0]);
        // if(likely(ABS(dq_curr[1]) < 1))dq_curr_[1] = LPFN<7>(dq_curr_[1],dq_curr[1]);

        dq_curr_[0] = LPFN<7>(dq_curr_[0],dq_curr[0]);
        dq_curr_[1] = LPFN<7>(dq_curr_[1],dq_curr[1]);
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