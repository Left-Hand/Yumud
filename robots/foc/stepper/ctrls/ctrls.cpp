#include "ctrls.hpp"

#define POS_ERR_LIMIT 100

using Result = CtrlResult;

scexpr real_t still_spd_radius = real_t(0.7);


void MetaData::reset(){
    pos_limit = Range::INF;
    max_curr = real_t(1.4);
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
    kp = 10;
    kp_limit = 40;

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


CtrlResult CurrentCtrl::update(const CtrlResult res){
    last_curr = STEP_TO(last_curr, res.current, config.curr_slew_rate);
    last_raddiff = STEP_TO(last_raddiff, res.raddiff, config.rad_slew_rate);
    // ctrl_done = ( == res.current);
    return {last_curr, last_raddiff};
}


Result PositionCtrl::update(real_t targ_pos, const real_t real_pos, 
    const real_t real_spd, const real_t real_elecrad){

    targ_pos = meta.pos_limit.clamp(targ_pos);
    targ_spd = (targ_spd * 127 + targ_spd_est.update(targ_pos)) >> 7;

    real_t pos_err = CLAMP2(targ_pos - real_pos, POS_ERR_LIMIT);
    real_t abs_pos_err = ABS(pos_err);
    real_t abs_real_spd = ABS(real_spd);
    real_t min_curr = meta.max_curr * real_t(0.2);
    
    scexpr real_t inverse_spd_thd = real_t(0.8);
    scexpr real_t stray_pos_thd = real_t(0.04);

    if(pos_err > 0 and real_spd < - inverse_spd_thd){//inverse run
        return {STEP_TO(meta.curr, min_curr, curr_ctrl.config.curr_slew_rate), 0};
    }

    if(pos_err < 0 and real_spd > inverse_spd_thd){//inverse run
        return {STEP_TO(meta.curr, min_curr, curr_ctrl.config.curr_slew_rate), 0};
    }

    if((abs_real_spd < inverse_spd_thd) and (abs_pos_err > stray_pos_thd)){
        return {meta.max_curr, SIGN_AS(pi_2, pos_err)};
    }

    {
        scexpr real_t inquater_radius = (inv_poles / 4);

        #define SAFE_OVERLOAD_RAD(__curr,__spd,__pos_abs_err)\
                MIN(\
                __curr * real_t(0.7) + real_t(0.3)\
                ,meta.max_leadrad)\

        real_t kp_contribute = abs_pos_err * config.kp;
        ki_integral = MIN(ki_integral + ((abs_pos_err * config.ki) >> 16), min_curr);

        real_t abs_curr = MIN(kp_contribute + ki_integral,meta.max_curr);

        // w = mv^2/2 - fx
        real_t overflow_energy;
        
        {
            scexpr auto ratio = 2;
            const real_t fixed_self_spd = (abs_real_spd + 12);
            // const real_t fixed_self_spd = (abs_real_spd);
            const real_t self_energy = fixed_self_spd * fixed_self_spd;
            // const real_t fixed_targ_spd = MAX(ABS(targ_spd) - 8, real_t(0));
            const real_t fixed_targ_spd =ABS(targ_spd);
            const real_t targ_energy = fixed_targ_spd * fixed_targ_spd;
            if(SIGN_DIFF(real_spd, targ_spd)){
                overflow_energy = MAX(
                        self_energy + targ_energy
                        - ratio * meta.max_acc * abs_pos_err
                        , 0);

            }else{
                overflow_energy = MAX(
                         self_energy - targ_energy
                        - ratio * meta.max_acc * abs_pos_err
                        , 0);
            }
        }

        abs_curr = MAX(abs_curr * MAX((1 - ((config.kd * overflow_energy) >> 8)), 0), min_curr);

        if(abs_pos_err < inquater_radius){
            return {abs_curr, pos_err * (poles * tau)};
        }else{
            real_t abs_raddiff = (1 + SAFE_OVERLOAD_RAD(meta.curr, abs_real_spd, abs_pos_err)) * hpi;
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

Result SpeedCtrl::update(real_t _targ_spd, real_t real_spd){

    const real_t clamped_targ_spd = CLAMP2(_targ_spd, meta.max_spd);
    
    soft_targ_spd = STEP_TO(soft_targ_spd, clamped_targ_spd, real_t(meta.max_acc) / foc_freq);
    if(ABS(soft_targ_spd) > 5 and ABS(real_spd) < real_t(0.5)) soft_targ_spd = 0;
    filt_real_spd = real_spd;

    const real_t spd_err = (soft_targ_spd - filt_real_spd);

    const real_t spd_delta = (filt_real_spd - last_real_spd);
    last_real_spd = filt_real_spd;

    const real_t kp_contribute = CLAMP2(spd_err * config.kp, config.kp_limit);
    const real_t kd_contribute = CLAMP2(config.kd * spd_delta, config.kd_limit);

    real_t delta_targ_curr = 0 
            + (kp_contribute >> 16)
            - (kd_contribute >> 8)
            ;

    delta_targ_curr = CLAMP2(delta_targ_curr, curr_ctrl.config.curr_slew_rate);

    real_t targ_current = SIGN_AS(meta.curr, soft_targ_spd) + delta_targ_curr;
    real_t abs_targ_current = MIN(ABS(targ_current), meta.max_curr);
    targ_current = SIGN_AS(abs_targ_current, soft_targ_spd);

    #define SAFE_OVERLOAD_RAD(__curr)\
        MIN(__curr * real_t(0.7) + real_t(0.3), meta.max_leadrad)


    const real_t abs_spd = ABS(filt_real_spd);
    real_t raddiff = SIGN_AS(hpi * (1 + (SAFE_OVERLOAD_RAD(meta.curr))), soft_targ_spd);

    bool is_inversed = false;
    is_inversed |= filt_real_spd * soft_targ_spd < -1;
    is_inversed |= (abs_spd > ABS(soft_targ_spd) * 2);

    if(is_inversed){
        // return {ctrl_meta.max_curr, SIGN_AS(basic_raddiff * PI / 2, targ_spd)};
        return {0, SIGN_AS(hpi, soft_targ_spd)};
    }else{
        return {abs_targ_current, raddiff};
    }

    #undef SAFE_OVERLOAD_RAD
}


Result TrapezoidPosCtrl::update(const real_t targ_pos,const real_t real_pos, const real_t real_spd, const real_t real_elecrad){

    const real_t max_spd = meta.max_spd;

    const real_t pos_err = targ_pos - real_pos;
    const real_t abs_pos_err = ABS(pos_err);
    
    real_t into_dec_radius = real_spd * real_spd / (2 * meta.max_acc);
    bool cross = last_pos_err * pos_err < 0;
    last_pos_err = pos_err;

    switch(tstatus){
        case Tstatus::ACC:
            if(abs_pos_err < into_dec_radius){
                tstatus = Tstatus::DEC;
            }

            {
                return speed_ctrl.update(SIGN_AS(max_spd, pos_err), real_spd);
            }
            break;

        case Tstatus::DEC:
            if(abs_pos_err < config.pos_sw_radius or cross){
                tstatus = Tstatus::STA;
            }

            if(abs_pos_err > into_dec_radius){
                tstatus = Tstatus::ACC;
            }

            {
                return speed_ctrl.update(0, real_spd);
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

// Result TrapezoidPosCtrl::update(real_t targ_pos, const real_t real_pos, const real_t real_spd, const real_t real_elecrad){

//     const real_t spd_acc_delta = meta.max_acc / foc_freq;
//     const real_t max_spd = meta.max_spd;

//     targ_pos = meta.pos_limit.clamp(targ_pos);
//     const real_t pos_err = CLAMP2(targ_pos - real_pos, POS_ERR_LIMIT);
//     const real_t abs_pos_err = ABS(pos_err);
    
//     real_t into_dec_radius = real_spd * real_spd / (2 * meta.max_acc);
//     bool cross = SIGN_DIFF(pos_err, last_pos_err);
//     last_pos_err = pos_err;

//     switch(tstatus){
//         case Tstatus::ACC:
//             if(abs_pos_err < into_dec_radius * 4){
//                 tstatus = Tstatus::DEC;
//             }

//             {
//                 goal_speed += SIGN_AS(spd_acc_delta, pos_err);
//                 goal_speed = CLAMP(goal_speed, -max_spd, max_spd);
//                 return speed_ctrl.update(real_pos, goal_speed, real_spd);
//             }
//             break;

//         case Tstatus::DEC:
//             if(abs_pos_err < config.pos_sw_radius or cross){
//                 tstatus = Tstatus::STA;
//             }

//             {
//                 goal_speed = SIGN_AS(sqrt(2 * meta.max_acc * abs_pos_err), pos_err);
//                 return speed_ctrl.update(real_pos, goal_speed, real_spd);
//             }
        
//             break;
//         default:
//         case Tstatus::STA:
//             if(abs_pos_err > config.pos_sw_radius){
//                 goal_speed = real_spd;
//                 tstatus = Tstatus::ACC;
//             }

//             return position_ctrl.update(targ_pos, real_pos, real_spd, real_elecrad);

//             break;
//     }
    
// }