#include "ctrls.hpp"

using Result = CtrlResult;

static constexpr real_t basic_raddiff = real_t(1.0);
static constexpr real_t max_raddiff = real_t(2.0);
static constexpr real_t POS_ERR_LIMIT = real_t(100);


void CtrlLimits::reset(){
    pos_limit = Range::INF;
    max_curr = real_t(1.2);
    max_spd = 30;
    max_acc = 30;
}

void PositionCtrl::Config::reset(){
    // kp = 5;
    // kd = real_t(0.57);
    kp = real_t(6.6);
    ki = real_t(100);
    kd = real_t(0.27);
    // kd = real_t(7);
}

void SpeedCtrl::Config::reset(){
    kp = 5;
    kp_limit = 4;

    kd = 10;
    kd_limit = real_t(7.4);
}

void TrapezoidPosCtrl::Config::reset(){
    pos_sw_radius = real_t(0.12);
}

void CurrentCtrl::Config::reset(){
    curr_slew_rate = real_t(60) / foc_freq;
    rad_slew_rate = real_t(420) / foc_freq;
    openloop_curr = real_t(0.7);
}


CtrlResult CurrentCtrl::update(const CtrlResult & res){
    curr_output = STEP_TO(curr_output, res.current, config.curr_slew_rate);
    raddiff_output = STEP_TO(raddiff_output, res.raddiff, config.rad_slew_rate);
    ctrl_done = (curr_output == res.current);
    return {curr_output, raddiff_output};
}


Result PositionCtrl::update(real_t targ_pos, const real_t real_pos, 
    const real_t real_spd, const real_t real_elecrad){

    targ_pos = limits.pos_limit.clamp(targ_pos);
    targ_spd = (targ_spd * 127 + targ_spd_est.update(targ_pos)) >> 7;
    // real_t abs_targ_spd = ABS(targ_spd);

    real_t pos_err = CLAMP2(targ_pos - real_pos, POS_ERR_LIMIT);
    real_t abs_pos_err = ABS(pos_err);
    real_t abs_real_spd = ABS(real_spd);
    real_t min_curr = limits.max_curr * real_t(0.1);
    
    static constexpr real_t inverse_spd_thd = real_t(0.8);
    static constexpr real_t stray_pos_thd = real_t(0.04);

    if(pos_err > 0 and real_spd < - inverse_spd_thd){//inverse run
        return {STEP_TO(curr_ctrl.getLastCurrent(), min_curr, curr_ctrl.config.curr_slew_rate), 0};
    }

    if(pos_err < 0 and real_spd > inverse_spd_thd){//inverse run
        return {STEP_TO(curr_ctrl.getLastCurrent(), min_curr, curr_ctrl.config.curr_slew_rate), 0};
    }

    if(abs_real_spd < inverse_spd_thd and abs_pos_err > stray_pos_thd){
        return {limits.max_curr, SIGN_AS(pi_2, pos_err)};
    }

    {
        static constexpr real_t inquater_radius = (inv_poles / 4);

        #define SAFE_OVERLOAD_RAD(__curr,__spd,__pos_abs_err)\
                MIN(\
                __curr * __curr * real_t(0.4) + real_t(0.2)\
                ,max_raddiff - basic_raddiff)\

        real_t kp_contribute = abs_pos_err * config.kp;
        ki_integral = MIN(ki_integral + ((abs_pos_err * config.ki) >> 16), min_curr);

        real_t abs_curr = MIN(kp_contribute + ki_integral,limits.max_curr);

        // w = mv^2/2 - fx
        real_t overflow_energy;
        
        {
            static constexpr auto ratio = 2;
            const real_t fixed_self_spd = (abs_real_spd + 12);
            // const real_t fixed_self_spd = (abs_real_spd);
            const real_t self_energy = fixed_self_spd * fixed_self_spd;
            // const real_t fixed_targ_spd = MAX(ABS(targ_spd) - 8, real_t(0));
            const real_t fixed_targ_spd =ABS(targ_spd);
            const real_t targ_energy = fixed_targ_spd * fixed_targ_spd;
            if(real_spd * targ_spd >= 0){
                overflow_energy = MAX(
                         self_energy - targ_energy
                        - ratio * limits.max_acc * abs_pos_err
                        , 0);
            }else{
                overflow_energy = MAX(
                        self_energy + targ_energy
                        - ratio * limits.max_acc * abs_pos_err
                        , 0);
            }
        }

        abs_curr = MAX(abs_curr * MAX((1 - ((config.kd * overflow_energy) >> 8)), 0), min_curr);

        if(abs_pos_err < inquater_radius){
            return {abs_curr, pos_err * (poles * tau)};
        }else{
            real_t abs_raddiff = (basic_raddiff + SAFE_OVERLOAD_RAD(curr_ctrl.curr_output, abs_real_spd, abs_pos_err)) * hpi;
            // real_t abs_raddiff = (basic_raddiff) * hpi;
            real_t raddiff = SIGN_AS(abs_raddiff, pos_err);
            return {abs_curr, raddiff};
        }
        #undef SAFE_OVERLOAD_RAD
    }


    //     return {0, 0};
    // }else{
    //     return {current, raddiff};
    // }
}

Result SpeedCtrl::update(const real_t _targ_spd,const real_t real_spd){

    const real_t clamped_targ_spd = CLAMP2(_targ_spd, limits.max_spd);
    
    soft_targ_spd = STEP_TO(soft_targ_spd, clamped_targ_spd, real_t(limits.max_acc) / foc_freq);
    filt_real_spd = (filt_real_spd * 63 + real_spd) >> 6;

    const real_t spd_err = (soft_targ_spd - filt_real_spd);

    const real_t spd_delta = (filt_real_spd - last_real_spd);
    last_real_spd = filt_real_spd;

    const real_t kp_contribute = CLAMP2(spd_err * config.kp, config.kp_limit);
    const real_t kd_contribute = CLAMP2(config.kd * spd_delta, config.kd_limit);

    real_t delta_targ_curr = 0 
            + (kp_contribute >> 16)
            - (kd_contribute >> 8)
            ;

    {
        const real_t delta_targ_curr_limit = curr_ctrl.config.curr_slew_rate;
        delta_targ_curr = CLAMP(delta_targ_curr, -delta_targ_curr_limit, delta_targ_curr_limit);
    }

    real_t targ_current = SIGN_AS(curr_ctrl.getLastCurrent(), last_real_spd) + delta_targ_curr;
    real_t abs_targ_current = MIN(ABS(targ_current), limits.max_curr);
    targ_current = SIGN_AS(abs_targ_current, soft_targ_spd);

    #define SAFE_OVERLOAD_RAD(__curr)\
        MIN(__curr * __curr * real_t(0.4) + real_t(0.2), max_raddiff - basic_raddiff)


    const real_t abs_spd = ABS(filt_real_spd);
    real_t raddiff = SIGN_AS(hpi * (basic_raddiff + (SAFE_OVERLOAD_RAD(curr_ctrl.curr_output))), soft_targ_spd);

    bool is_inversed = false;
    is_inversed |= filt_real_spd * soft_targ_spd < -1;
    is_inversed |= (abs_spd > ABS(soft_targ_spd) * 2);

    if(is_inversed){
        // return {ctrl_limits.max_curr, SIGN_AS(basic_raddiff * PI / 2, targ_spd)};
        return {0, SIGN_AS(basic_raddiff * hpi, soft_targ_spd)};
    }else{
        return {abs_targ_current, raddiff};
    }

    #undef SAFE_OVERLOAD_RAD
}




Result TrapezoidPosCtrl::update(const real_t targ_pos,const real_t real_pos, const real_t real_spd, const real_t real_elecrad){

    const real_t spd_acc_delta = real_t(limits.max_acc) / foc_freq;
    const real_t max_spd = limits.max_spd;

    const real_t pos_err = targ_pos - real_pos;
    const real_t abs_pos_err = ABS(pos_err);
    
    real_t into_dec_radius = real_spd * real_spd / (2 * limits.max_acc);
    bool cross = last_pos_err * pos_err < 0;
    last_pos_err = pos_err;

    switch(tstatus){
        case Tstatus::ACC:
            if(abs_pos_err < into_dec_radius){
                tstatus = Tstatus::DEC;
            }

            {
                goal_speed += SIGN_AS(spd_acc_delta, pos_err);
                goal_speed = CLAMP(goal_speed, -max_spd, max_spd);
                return speed_ctrl.update(goal_speed, real_spd);
            }
            break;

        case Tstatus::DEC:
            // if(abs_pos_err > into_dec_radius){
            //     tstatus = Tstatus::ACC;
            // }
            if(abs_pos_err < config.pos_sw_radius or cross){
            // if(cross){
                tstatus = Tstatus::STA;
            }

                // real_t max_exp_spd = sqrt(2 * config.max_acc * abs_pos_err);
            {
                // goal_speed -= SIGN_AS(spd_dec_delta, pos_err);
                // goal_speed = CLAMP(goal_speed, -max_exp_spd, max_exp_spd);

                // goal_speed = STEP_TO(goal_speed, SIGN_AS(sqrt(2 * config.max_acc * abs_pos_err), pos_err), spd_dec_delta);
                goal_speed = SIGN_AS(sqrt(2 * limits.max_acc * abs_pos_err), pos_err);
                return speed_ctrl.update(goal_speed, real_spd);
            }
        
            break;
        default:
        case Tstatus::STA:
            if(abs_pos_err > config.pos_sw_radius){
                goal_speed = real_spd;
                tstatus = Tstatus::ACC;
            }

            return position_ctrl.update(targ_pos, real_pos, real_spd, real_elecrad);

            break;
    }
    
}