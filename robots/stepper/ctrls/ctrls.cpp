#include "ctrls.hpp"

using Result = CtrlResult;


real_t CurrentCtrl::update(const real_t targ_current){
    real_t current_delta = CLAMP(targ_current - current_output, -current_slew_rate, current_slew_rate);
    return current_output = MIN(current_output + current_delta, current_clamp);
}

Result GeneralPositionCtrl::update(const real_t targ_pos, const real_t real_pos, 
        const real_t real_spd, const real_t real_elecrad){

    real_t err = targ_pos - real_pos;
    real_t abs_err = ABS(err);
    real_t abs_spd = ABS(real_spd);

    static constexpr real_t basic_raddiff = 1.0;

    real_t clamped_abs_err = MIN(abs_err, (inv_poles / 4) * PI / 2);
    real_t smoothed_raw_abs_raddiff = sin(clamped_abs_err * poles * 4) * (PI/2);

    #define SAFE_OVERLOAD_RAD(__curr, __spd, __pos_abs_err)\
        MIN(\
        MIN(\
        MIN(__curr * __curr\
        , __spd * 0.10),\
        \
        __pos_abs_err * 5)\
        ,1.7)
    real_t raddiff = smoothed_raw_abs_raddiff * SIGN_AS((basic_raddiff + SAFE_OVERLOAD_RAD(curr_ctrl.current_output, abs_spd, abs_err)), err);

    real_t abs_curr = MIN(abs_err * kp, curr_ctrl.config.curr_limit); 

    //w = mv^2/2 - fx
    real_t overflow_energy = MAX(kd * abs_spd - kd2 * sqrt(abs_err), 0); 
    
    abs_curr = MAX(abs_curr - overflow_energy, 0);
    
    if(err * real_spd < -2){
        return {curr_ctrl.config.curr_limit, SIGN_AS(basic_raddiff, err)};
    }else{
        return {abs_curr, raddiff};
    }
}

Result GeneralSpeedCtrl::update(const real_t _targ_speed,const real_t real_speed){
    const real_t targ_speed = CLAMP(_targ_speed, -max_spd, max_spd);
    real_t error = (targ_speed - real_speed);

    real_t speed_delta = real_speed - last_speed;
    last_speed = real_speed; 

    real_t kp_contribute = CLAMP(error * kp, -kp_clamp, kp_clamp);
    real_t kd_contribute = CLAMP(kd * speed_delta, -kd_clamp, kd_clamp);

    if((ABS(error) > kd_active_radius)) kd_contribute = 0;

    targ_current += (kp_contribute >> 16);
    targ_current -= (kd_contribute >> 8); 

    real_t abs_targ_current = MIN(ABS(targ_current), curr_ctrl.config.curr_limit);
    targ_current = SIGN_AS(abs_targ_current, targ_speed);

    if(real_speed * targ_speed > 0) return {abs_targ_current, SIGN_AS(((PI / 2) * (1 + MIN(curr_ctrl.current_output, 1.2))), targ_speed)};
    else return {abs_targ_current, SIGN_AS(PI / 2, targ_speed)};
}


Result TrapezoidPosCtrl::update(const real_t targ_pos,const real_t real_pos, const real_t real_spd, const real_t real_elecrad){
    static constexpr real_t hug_speed = 1.3;

    const real_t spd_delta = max_dec / foc_freq;
    const real_t max_spd = speed_ctrl.max_spd;

    const real_t pos_err = targ_pos - real_pos;
    const real_t abs_pos_err = ABS(pos_err);
    bool cross = pos_err * last_pos_err < 0;
    last_pos_err = pos_err;
    
    switch(tstatus){
        case Tstatus::ACC:
            if(real_spd * real_spd > 2 * max_dec * abs_pos_err){
                tstatus = Tstatus::DEC;
            }

            {
                goal_speed += SIGN_AS(spd_delta, pos_err);
                goal_speed = CLAMP(goal_speed, -max_spd, max_spd);
                return speed_ctrl.update(goal_speed, real_spd);
            }
            break;

        case Tstatus::DEC:
            goal_speed = SIGN_AS(sqrt(2 * max_dec * ABS(pos_err)), pos_err);

            if((cross and (ABS(real_spd) < hug_speed)) and abs_pos_err < pos_sw_radius){
                tstatus = Tstatus::STA;
            }
            // if(cross){
            //     tstatus = Tstatus::STA;
            // }

            {
                // bool ovs = real_speed * real_speed > 2 * max_dec* ABS(pos_err);
                // goal_speed = SIGN_AS(sqrt(2 * max_dec* ABS(pos_err)), goal_speed);
                // if(ovs) goal_speed += SIGN_AS(-spd_delta, pos_err);
                // if(ovs) goal_speed += -spd_delta;
                // else goal_speed += SIGN_AS(spd_delta / 3, pos_err);

                // if(pos_err > 0) goal_speed = CLAMP(goal_speed, hug_speed, max_spd);
                // else goal_speed = CLAMP(goal_speed, -max_spd, -hug_speed);

                
                return speed_ctrl.update(goal_speed, real_spd);
            }

            break;
        default:
        case Tstatus::STA:
            if(ABS(pos_err) > pos_sw_radius){
                goal_speed = real_spd;
                tstatus = Tstatus::ACC;
            }
            return position_ctrl.update(targ_pos, real_pos, real_spd, real_elecrad);

            break;
    }
    
}