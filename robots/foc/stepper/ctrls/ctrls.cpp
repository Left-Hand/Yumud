#include "ctrls.hpp"

using namespace ymd::foc;

using MetaData = MotorUtils::MetaData;

#define ERR_LIMIT 100

void MetaData::reset(){
    pos_limit = Range2<real_t>::INF;
    max_curr = real_t(1.4);
    max_spd = 30;
    max_acc = 30;
    spd_to_leadrad_ratio = real_t(0.3);
    curr_to_leadrad_ratio = real_t(1.9);
    // max_leadrad = real_t(PI * 1.9);
    // max_leadrad = real_t(PI * 1);
    // max_leadrad = real_t(PI * 0.3);
    max_leadrad = real_t(PI * 0.45);
    // max_leadrad = real_t(0);
    openloop_curr = real_t(0.7);
}

CtrlResult PositionCtrl::update(
        real_t targ_pos, 
        const real_t real_pos, 
        const real_t real_spd)
    {

    static constexpr real_t inquater_radius = real_t(inv_poles / 4);


    real_t e1 = CLAMP2(targ_pos - real_pos, ERR_LIMIT);

    real_t e2 = CLAMP2(meta.targ_spd - real_spd, ERR_LIMIT);
    real_t abs_e1 = ABS(e1);

    real_t w_k_change =  (config.kd * e2) >> 8;

    real_t w_elapsed = config.kp * SIGN_AS(sqrt(abs_e1), e1);

    static constexpr auto ki = real_t(0.01);
    real_t signed_curr = CLAMP2(w_elapsed + w_k_change, meta.max_curr) + ki * e1;
    real_t curr = ABS(signed_curr);

    if(unlikely(abs_e1 < inquater_radius)){
        return {curr, e1 * (poles * tau)};
    }else{
        return {curr, SIGN_AS(meta.get_max_raddiff(), signed_curr)};
    }
}

CtrlResult SpeedCtrl::update(real_t _targ_spd, real_t real_spd){
    // const real_t clamped_targ_spd = CLAMP2(_targ_spd, meta.max_spd);
    
    // soft_targ_spd = STEP_TO(soft_targ_spd, clamped_targ_spd, real_t(meta.max_acc) / foc_freq);
    soft_targ_spd = _targ_spd;

    const real_t e2 = (soft_targ_spd - real_spd);
    spd_delta = real_spd - last_real_spd;
    // spd_delta = (spd_delta * 31 + (real_spd - last_real_spd)) >> 5;
    
    last_real_spd = real_spd;

    const real_t kp_contribute = e2 * config.kp;
    const real_t kd_contribute = spd_delta * config.kd;

    real_t delta_targ_curr = ((kp_contribute >> 8) - (kd_contribute));

    real_t targ_curr = CLAMP2(((SIGN_AS(meta.curr, soft_targ_spd) << 8) + delta_targ_curr) >> 8, meta.max_curr);

    return {ABS(targ_curr), SIGN_AS(meta.get_max_raddiff(), targ_curr)};   
}