#include "ctrls.hpp"

using Result = CtrlResult;

static constexpr real_t basic_raddiff = real_t(1.0);
static constexpr real_t max_raddiff = real_t(2.0);
static constexpr real_t tau = real_t(TAU);
static constexpr real_t pi = real_t(PI);
static constexpr real_t hpi = real_t(PI/2); 

void CtrlLimits::reset(){
    pos_limit = Range::INF;
    max_curr = real_t(1.2);
    max_spd = 30;
    max_acc = 30;
}

void PositionCtrl::Config::reset(){
    // kp = 5;
    // kd = real_t(0.57);
    kp = real_t(5.6);
    kd = real_t(0.22);
}

void SpeedCtrl::Config::reset(){
    kp = 12;
    kp_limit = 40;

    kd = 20;
    kd_limit = real_t(14.4);
}

void TrapezoidPosCtrl::Config::reset(){
    pos_sw_radius = real_t(0.12);
}

CtrlResult CurrentCtrl::update(CtrlResult res){
    current_output = STEP_TO(current_output, res.current, config.curr_slew_rate);
    raddiff_output = STEP_TO(raddiff_output, res.raddiff, config.rad_slew_rate);
    return {current_output, raddiff_output};
}

void CurrentCtrl::Config::reset(){
    curr_slew_rate = real_t(60) / foc_freq;
    rad_slew_rate = real_t(70) / foc_freq;
    openloop_curr = real_t(0.7);
}

Result PositionCtrl::update(const real_t targ_pos, const real_t real_pos, 
    const real_t real_spd, const real_t real_elecrad){

    targ_spd = (targ_spd * 127 + targ_spd_est.update(targ_pos)) >> 7;
    real_t abs_targ_spd = ABS(targ_spd);

    real_t pos_err = targ_pos - real_pos;
    
    if(pos_err * real_spd < -2){//inverse run
        return {limits.max_curr, SIGN_AS(basic_raddiff * hpi, pos_err)};
    }else{
        real_t abs_pos_err = ABS(pos_err);
        real_t abs_spd = ABS(real_spd);

        static constexpr real_t inquater_radius = (inv_poles / 4);

        if(abs_pos_err < inquater_radius){
            real_t raddiff = pos_err * poles * tau;
            real_t abs_curr = config.kp * inquater_radius;

            return {abs_curr, raddiff};
        }else{
            #define SAFE_OVERLOAD_RAD(__curr,__spd,__pos_abs_err)\
                    MIN(\
                    __curr * __curr * real_t(0.4) + real_t(0.2)\
                    ,max_raddiff - basic_raddiff)\

            real_t abs_raddiff = (basic_raddiff + SAFE_OVERLOAD_RAD(curr_ctrl.current_output, abs_spd, abs_pos_err)) * hpi;
            real_t raddiff = SIGN_AS(abs_raddiff, pos_err);

            real_t abs_curr = MIN(
                abs_pos_err * config.kp
                ,limits.max_curr
                ,abs_raddiff
                ); 

            // w = mv^2/2 - fx

            real_t overflow_energy = MAX((config.kd >> 8) * (
                    (abs_spd + targ_spd) * (abs_spd - targ_spd) * SIGN(abs_spd - abs_targ_spd)
                    - 2 * limits.max_acc * MIN(abs_pos_err, 100)), 0);
                    // - 2 * limits.max_acc * abs_pos_err, 0);



            abs_curr = MAX(abs_curr - overflow_energy, 0);
            return {abs_curr, raddiff};
            
            #undef SAFE_OVERLOAD_RAD
        }

    }


    //     return {0, 0};
    // }else{
    //     return {current, raddiff};
    // }
}

Result SpeedCtrl::update(const real_t _targ_spd,const real_t real_spd){

    const real_t targ_spd = CLAMP2(_targ_spd, limits.max_spd);
    const real_t spd_err = (targ_spd - real_spd);

    const real_t spd_delta = real_spd - last_speed;
    last_speed = real_spd;

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

    targ_current += delta_targ_curr;
    real_t abs_targ_current = MIN(ABS(targ_current), limits.max_curr);
    targ_current = SIGN_AS(abs_targ_current, targ_spd);

    #define SAFE_OVERLOAD_RAD(__curr, __spd)\
        MIN(__curr * __curr * real_t(0.4), __spd * real_t(0.17) , max_raddiff - basic_raddiff)


    const real_t abs_spd = ABS(real_spd);
    real_t raddiff = SIGN_AS(hpi * (basic_raddiff + (SAFE_OVERLOAD_RAD(curr_ctrl.current_output, abs_spd))), targ_spd);

    bool is_inversed = false;
    is_inversed |= real_spd * targ_spd < -1;
    is_inversed |= (abs_spd > ABS(targ_spd) * 2);

    if(is_inversed){
        // return {ctrl_limits.max_curr, SIGN_AS(basic_raddiff * PI / 2, targ_spd)};
        return {0, SIGN_AS(basic_raddiff * hpi, targ_spd)};
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