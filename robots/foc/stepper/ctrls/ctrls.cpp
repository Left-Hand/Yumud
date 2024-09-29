#include "ctrls.hpp"

#define ERR_LIMIT 200


using Result = CtrlResult;


void MetaData::reset(){
    pos_limit = Range::INF;
    max_curr = real_t(1.4);
    max_spd = 30;
    max_acc = 30;
    spd_to_leadrad_ratio = real_t(0.3);
    curr_to_leadrad_ratio = real_t(1.9);
    max_leadrad = real_t(PI * 1.9);
    openloop_curr = real_t(0.7);
}

void CurrentCtrl::Config::reset(){
    curr_slew_rate = real_t(60) / foc_freq;
    rad_slew_rate = real_t(30) / foc_freq;
}

void PositionCtrl::Config::reset(){
    kp = real_t(1.5);
    kd = real_t(9.2);
}

void SpeedCtrl::Config::reset(){
    kp = 16;
    kp_limit = 40;

    kd = 16;
    kd_limit = real_t(14.4);
}

Result PositionCtrl::update(real_t targ_pos, const real_t real_pos, 
    const real_t real_spd){

    scexpr real_t inquater_radius = real_t(inv_poles / 4);
    scexpr real_t min_curr = real_t(0.01);
        
    targ_spd = (targ_spd * 127 + targ_spd_est.update(targ_pos)) >> 7;

    real_t pos_err = CLAMP2(targ_pos - real_pos, ERR_LIMIT);

    real_t spd_err = CLAMP2(targ_spd - real_spd, ERR_LIMIT);
    real_t abs_pos_err = ABS(pos_err);

    real_t w_k_change =  (config.kd * spd_err) >> 8;

    real_t w_elapsed = config.kp * SIGN_AS(sqrt(ABS(pos_err)), pos_err);

    real_t w = CLAMP2(w_elapsed + w_k_change, meta.max_curr);
    real_t curr = MAX(ABS(w), min_curr);

    if(unlikely(abs_pos_err < inquater_radius)){
        return {curr, pos_err * (poles * tau)};
    }else{
        return {curr, SIGN_AS(meta.get_max_raddiff(), w)};
    }
}

Result SpeedCtrl::update(real_t _targ_spd, real_t real_spd){
    const real_t clamped_targ_spd = CLAMP2(_targ_spd, meta.max_spd);
    
    soft_targ_spd = STEP_TO(soft_targ_spd, clamped_targ_spd, real_t(meta.max_acc) / foc_freq);

    const real_t spd_err = (soft_targ_spd - real_spd);
    const real_t spd_delta = (real_spd - last_real_spd);
    last_real_spd = real_spd;

    const real_t kp_contribute = CLAMP2(spd_err * config.kp, config.kp_limit);
    const real_t kd_contribute = CLAMP2(spd_delta * config.kd, config.kd_limit);

    real_t delta_targ_curr = (kp_contribute >> 16) - (kd_contribute >> 8);

    real_t targ_curr = CLAMP2(SIGN_AS(meta.curr, soft_targ_spd) + delta_targ_curr, meta.max_curr);

    bool dir_correct = meta.spd * soft_targ_spd >= 0; 

    if(dir_correct){
        return {ABS(targ_curr), SIGN_AS(meta.get_max_raddiff(), targ_curr)};   
    }else{
        return {ABS(targ_curr) * MAX(real_t(1) - meta.spd * real_t(0.1), real_t(0)), SIGN_AS(real_t(PI/2), targ_curr)};
    }
}