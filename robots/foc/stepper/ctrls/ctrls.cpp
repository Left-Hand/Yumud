#include "ctrls.hpp"

#define POS_ERR_LIMIT 100

#define SPD_CTRL_TYPE_ORG 0
#define SPD_CTRL_TYPE_BANGBANG 1

#define SPD_CTRL_TYPE SPD_CTRL_TYPE_ORG

using Result = CtrlResult;

// scexpr real_t still_spd_radius = real_t(0.7);


void MetaData::reset(){
    pos_limit = Range::INF;
    max_curr = real_t(1.4);
    max_spd = 30;
    max_acc = 30;
    spd_to_leadrad_ratio = real_t(0.3);
    curr_to_leadrad_ratio = real_t(1.4);
    max_leadrad = real_t(PI * 0.7);
}

void CurrentCtrl::Config::reset(){
    curr_slew_rate = real_t(60) / foc_freq;
    rad_slew_rate = real_t(420) / foc_freq;
    openloop_curr = real_t(0.7);
}

void PositionCtrl::Config::reset(){
    // kp = 5;
    // kd = real_t(0.57);
    kp = real_t(6.6);
    ki = real_t(100);
    kd = real_t(0.27);
    // kd = real_t(0);
}

void SpeedCtrl::Config::reset(){
    kp = 16;
    kp_limit = 40;

    kd = 16;
    kd_limit = real_t(14.4);
}

void TrapezoidPosCtrl::Config::reset(){
    pos_sw_radius = real_t(0.12);
}


CtrlResult CurrentCtrl::update(const CtrlResult res){
    last_curr = STEP_TO(last_curr, res.current, config.curr_slew_rate);
    last_raddiff = STEP_TO(last_raddiff, res.raddiff, config.rad_slew_rate);

    return {last_curr, last_raddiff};
}


Result PositionCtrl::update(real_t targ_pos, const real_t real_pos, 
    const real_t real_spd, const real_t real_elecrad){

    targ_pos = meta.pos_limit.clamp(targ_pos);
    targ_spd = (targ_spd * 31 + targ_spd_est.update(targ_pos)) >> 5;

    real_t pos_err = CLAMP2(targ_pos - real_pos, POS_ERR_LIMIT);
    real_t abs_pos_err = ABS(pos_err);
    real_t abs_real_spd = ABS(real_spd);
    real_t abs_min_curr = meta.max_curr * real_t(0.2);
    
    scexpr real_t inverse_spd_thd = real_t(0.8);
    scexpr real_t stray_pos_thd = real_t(0.04);

    if(pos_err > 0 and real_spd < - inverse_spd_thd){//inverse run
        return {STEP_TO(meta.curr, abs_min_curr, curr_ctrl.config.curr_slew_rate), 0};
    }

    if(pos_err < 0 and real_spd > inverse_spd_thd){//inverse run
        return {STEP_TO(meta.curr, abs_min_curr, curr_ctrl.config.curr_slew_rate), 0};
    }

    if((abs_real_spd < inverse_spd_thd) and (abs_pos_err > stray_pos_thd)){
        return {meta.max_curr, SIGN_AS(pi_2, pos_err)};
    }

    {
        scexpr auto inquater_radius = real_t(inv_poles / 4);

        scexpr auto kd1 = real_t(0.001);
        scexpr auto kd2 = real_t(5.0);

        real_t w_k_change =  kd1 * (targ_spd * targ_spd - real_spd * real_spd);

        real_t w_elapsed = CLAMP2(kd2 * abs_pos_err, meta.max_curr);
        // real_t w_elapsed = kd2 * abs_pos_err;

        real_t w = CLAMP2(w_elapsed + w_k_change, meta.max_curr);

        real_t abs_curr = ABS(w);

        if(abs_pos_err < inquater_radius){
            return {abs_curr, pos_err * (poles * tau)};
        }else{
            real_t abs_raddiff = meta.get_max_raddiff();
            real_t raddiff = SIGN_AS(abs_raddiff, pos_err);
            return {abs_curr, raddiff};
        }
    }
}

Result SpeedCtrl::update(real_t _targ_spd, real_t real_spd){
    const real_t clamped_targ_spd = CLAMP2(_targ_spd, meta.max_spd);
    
    soft_targ_spd = STEP_TO(soft_targ_spd, clamped_targ_spd, real_t(meta.max_acc) / foc_freq);

    #if (SPD_CTRL_TYPE == SPD_CTRL_TYPE_ORG)

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
    
    #elif (SPD_CTRL_TYPE == SPD_CTRL_TYPE_BANGBANG)

    scexpr real_t bangbang_step = real_t(0.001); 

    real_t delta_targ_curr = CLAMP2(SIGN_AS(bangbang_step, soft_targ_spd - real_spd), curr_ctrl.config.curr_slew_rate);

    real_t raddiff = SIGN_AS(meta.get_max_raddiff(), soft_targ_spd);

    return {meta.curr + delta_targ_curr, raddiff};
    #endif

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