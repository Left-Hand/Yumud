#include "ctrls.hpp"

using Result = CtrlResult;

Result GeneralPositionCtrl::update(const real_t targ_pos, const real_t real_pos, 
        const real_t real_spd, const real_t real_elecrad){

    real_t err = targ_pos - real_pos;
    real_t abs_err = ABS(err);

    static constexpr double u = TAU;
    static constexpr double pu = poles * u;
    static constexpr double a = 2 / PI;
    static constexpr double k = a * a * pu;

    real_t abs_uni_raddiff = real_t(PI/2) - 1/(real_t(k) * abs_err + a);

    real_t raddiff = abs_uni_raddiff * SIGN_AS((1 + MIN(curr_ctrl.current_output, 0.45)) , err);

    real_t current = MIN(abs_err * kp, curr_ctrl.config.current_clamp); 

    if(abs_err < kd_active_radius){
        current = MAX(current - (kd * ABS(real_spd) >> 8), 0);
    }
    
    if(((err > 0) && (real_spd < -1)) || ((err < 0) && (real_spd > 1))){
        return {0, 0};
    }else{
        return {current, raddiff};
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

    real_t abs_targ_current = MIN(ABS(targ_current), curr_ctrl.config.current_clamp);
    targ_current = SIGN_AS(abs_targ_current, targ_speed);

    if(real_speed * targ_speed > 0) return {abs_targ_current, SIGN_AS((PI / 2 * (1 + MIN(curr_ctrl.current_output, 1.2))), targ_speed)};
    else return {abs_targ_current, SIGN_AS(PI / 2, targ_speed)};
}


Result TrapezoidPosCtrl::update(const real_t targ_position,const real_t real_position, const real_t real_speed, const real_t real_elecrad){
    real_t spd_delta = max_dec/foc_freq;
    real_t max_spd = speed_ctrl.max_spd;

    real_t pos_err = targ_position - real_position;
    bool cross = pos_err * last_pos_err < 0;
    last_pos_err = pos_err;
    switch(tstatus){
        case Tstatus::ACC:
            if(real_speed * real_speed > 2 * max_dec* ABS(pos_err)){
                tstatus = Tstatus::DEC;
            }

            {
                goal_speed += SIGN_AS(spd_delta, pos_err);
                goal_speed = CLAMP(goal_speed, -max_spd, max_spd);
                return speed_ctrl.update(goal_speed, real_speed);
            }
            break;

        case Tstatus::DEC:
            // if((cross and (ABS(goal_speed) < hug_speed)) || abs(pos_err) < pos_sw_radius){
            //     tstatus = Tstatus::STA;
            // }
            if(cross){
                tstatus = Tstatus::STA;
            }

            {
                // bool ovs = real_speed * real_speed > 2 * max_dec* ABS(pos_err);
                // goal_speed = SIGN_AS(sqrt(2 * max_dec* ABS(pos_err)), goal_speed);
                // if(ovs) goal_speed += SIGN_AS(-spd_delta, pos_err);
                // if(ovs) goal_speed += -spd_delta;
                // else goal_speed += SIGN_AS(spd_delta / 3, pos_err);

                // if(pos_err > 0) goal_speed = CLAMP(goal_speed, hug_speed, max_spd);
                // else goal_speed = CLAMP(goal_speed, -max_spd, -hug_speed);

                goal_speed = SIGN_AS(sqrt(2 * max_dec * ABS(pos_err)), pos_err);
                return speed_ctrl.update(goal_speed, real_speed);
            }

            break;
        default:
        case Tstatus::STA:
            if(ABS(pos_err) > pos_sw_radius){
                goal_speed = real_speed;
                tstatus = Tstatus::ACC;
            }
            return position_ctrl.update(targ_position, real_position, real_speed, real_elecrad);

            break;
    }
    
}