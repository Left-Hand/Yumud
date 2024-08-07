#include "ctrls.hpp"

using Result = CtrlResult;

static constexpr real_t basic_raddiff = 1.0;
static constexpr real_t max_raddiff = 2.3;

CtrlResult CurrentCtrl::update(CtrlResult res){
    current_output = STEP_TO(current_output, res.current, config.curr_slew_rate);
    raddiff_output = STEP_TO(raddiff_output, res.raddiff, config.rad_slew_rate);
    return {current_output, raddiff_output};
}

void CurrentCtrl::Config::reset(){
    curr_slew_rate = 60.0 / foc_freq;
    rad_slew_rate = 70.0 / foc_freq;
    curr_limit = 0.7;
    openloop_curr = 0.7;
}

Result GeneralPositionCtrl::update(const real_t targ_pos, const real_t real_pos, 
    const real_t real_spd, const real_t real_elecrad){

    real_t err = targ_pos - real_pos;

    
    if(err * real_spd < -2){//inverse run
        return {curr_ctrl.config.curr_limit, SIGN_AS(basic_raddiff * (PI/2), err)};
        // return {0,0};
    }else{
        real_t abs_err = ABS(err);
        real_t abs_spd = ABS(real_spd);

        static constexpr real_t inquater_radius = (inv_poles / 4) * PI / 2;


        if(abs_err < inquater_radius){
            // real_t raddiff = (PI/2) * basic_raddiff * (frac(frac(targ_pos) * poles) * 4) - real_elecrad;
            real_t raddiff = (PI/2) * basic_raddiff * (err * poles * 4);
            real_t abs_curr = kp * inquater_radius;

            return {abs_curr, raddiff};
        }else
        {


            auto SAFE_OVERLOAD_RAD = [](const real_t __curr,const real_t __spd,const real_t __pos_abs_err){
                return MIN(
                //     // 0.0,
                //     0.4,
                // __curr * 0.4,
                    __spd * 0.07,
                    __pos_abs_err * 2.2,
                    max_raddiff - basic_raddiff
                );
            };

            real_t raddiff = (PI/2) * SIGN_AS((basic_raddiff + SAFE_OVERLOAD_RAD(curr_ctrl.current_output, abs_spd, abs_err)), err);
            // real_t raddiff = real_t(PI/2) * SIGN_AS(1.2, err);

            real_t abs_curr = MIN(abs_err * kp, curr_ctrl.config.curr_limit); 

            // w = mv^2/2 - fx
            real_t overflow_energy = MAX(kd * abs_spd - kd2 * sqrt(abs_err), 0); 
            
            abs_curr = MAX(abs_curr - overflow_energy, 0);
            return {abs_curr, raddiff};
            
            #undef SAFE_OVERLOAD_RAD
        }

    }

}

Result GeneralSpeedCtrl::update(const real_t _targ_spd,const real_t real_spd){

    const real_t targ_spd = CLAMP(_targ_spd, -max_spd, max_spd);
    const real_t err = (targ_spd - real_spd);
    const real_t abs_err = ABS(err);

    const real_t spd_delta = real_spd - last_speed;
    last_speed = real_spd;

    const real_t kp_contribute = CLAMP(err * kp, -kp_clamp, kp_clamp);
    const real_t kd_contribute = (abs_err < kd_active_radius) ? CLAMP(kd * spd_delta, -kd_clamp, kd_clamp) : 0;

    real_t delta_targ_curr = 0 
            + (kp_contribute >> 16)
            - (kd_contribute >> 8)
            ; 

    {
        const real_t delta_targ_curr_limit = curr_ctrl.config.curr_slew_rate;
        delta_targ_curr = CLAMP(delta_targ_curr, -delta_targ_curr_limit, delta_targ_curr_limit);
    }
    targ_current += delta_targ_curr;
    real_t abs_targ_current = MIN(ABS(targ_current), curr_ctrl.config.curr_limit);
    targ_current = SIGN_AS(abs_targ_current, targ_spd);

    #define SAFE_OVERLOAD_RAD(__curr, __spd)\
        MIN(\
        MIN(__curr * __curr\
        , __spd * 0.10)\
        , max_raddiff)\


    const real_t abs_spd = ABS(real_spd);
    real_t raddiff = SIGN_AS((PI / 2) * (basic_raddiff + (SAFE_OVERLOAD_RAD(curr_ctrl.current_output, abs_spd))), real_spd);

    bool is_inverse = false;
    is_inverse |= real_spd * targ_spd < -1;
    is_inverse |= (abs_spd > ABS(targ_spd) * 2);
    if(is_inverse){
        // return {curr_ctrl.config.curr_limit, SIGN_AS(basic_raddiff * PI / 2, targ_spd)};
        return {0, SIGN_AS(basic_raddiff * PI / 2, targ_spd)};
    }else{
        return {abs_targ_current, raddiff};
    }

    #undef SAFE_OVERLOAD_RAD
}


void GeneralSpeedCtrl::Config::reset(){
    max_spd = 60;
    kp = 3;
    kp_clamp = 40;

    kd = 18;
    kd_active_radius = 2.7;
    kd_clamp = 14.4;
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