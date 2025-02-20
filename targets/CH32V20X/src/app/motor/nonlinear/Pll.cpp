#include "Pll.hpp"

using namespace ymd::foc;

void SimplePll::update(const iq_t lap_pos){
    scexpr iq_t kp_ = 0.17_r;
    scexpr iq_t ki_ = 0.0027_r;
    //  .fc = 10000}

    iq_t delta_lap_pos = lap_pos - last_lap_pos;

    if(delta_lap_pos >= iq_t(0.5)){
        delta_lap_pos -= 1;
    }else if (delta_lap_pos <= -iq_t(0.5)){
        delta_lap_pos += 1;
    }

    accu_pos_ += delta_lap_pos;
    last_lap_pos = lap_pos;
    
    iq_t pos_err = accu_pos_ - pll_pos_;
    err_int_ += (pos_err);
    pll_pos_ += (err_int_ * ki_ + pos_err * kp_);
}

iq_t LapPosPll::update(const iq_t lap_pos){
    const iq_t pos_meas = pos_accumulator_.update(lap_pos);
    const iq_t pos_err = pos_meas - pos_est_;

    pos_err_int_ += pos_err;
    spd_est_ = kp_ * pos_err + ki_ * pos_err_int_;
    pos_int_ += spd_est_ * ko_;

    // spd_est_ = spd_pi_ctrl_.update(pos_err);
    pos_est_ = pos_int_ >> shift_bits;

    // return spd_est_;
    return pos_est_;
    // return pos_meas;
}