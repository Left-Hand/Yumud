#include "robots/foc/stepper/stepper.hpp"


void FOCStepper::active_task(){
    scexpr auto ratio = real_t(0.5);

    if(ctrl_type == CtrlType::VECTOR){
        run_elecrad = odo.position2rad(target);
        svpwm.setDuty(curr_ctrl.config.openloop_curr * ratio, run_elecrad + elecrad_zerofix);
    }else{
        run_elecrad = est_elecrad + curr_ctrl.raddiff();
        svpwm.setDuty(curr_ctrl.curr() * ratio, run_elecrad + elecrad_zerofix);
    }

    odo.update();

    meta.pos = odo.getPosition();
    est_elecrad = odo.getElecRad();
    meta.spd = (speed_estmator.update(meta.pos) + meta.spd * 127) >> 7;

    {
        using Result = CtrlResult;
        Result result;

        const auto & est_pos = meta.pos;
        const auto & est_spd = meta.spd;

        switch(ctrl_type){
            case CtrlType::CURRENT:
                result = {ABS(target), SIGN_AS(real_t(PI / 2) *real_t(1.3), target)};
                break;
            case CtrlType::VECTOR:
                result = {meta.max_curr, 0};
                break;

            case CtrlType::POSITION:
                result = position_ctrl.update(target, est_pos, est_spd, est_elecrad);
                break;
            case CtrlType::TRAPEZOID:
                result = trapezoid_ctrl.update(target, est_pos, est_spd, est_elecrad);
                break;
    
            case CtrlType::SPEED:{
                scexpr real_t dead_zone = real_t(0.003);
                if((est_pos >= meta.pos_limit.to - dead_zone and target > 0)
                     or (est_pos <= meta.pos_limit.from + dead_zone and target < 0)){
                    result = position_ctrl.update((target > 0 ? meta.pos_limit.to : meta.pos_limit.from)
                            , est_pos, est_spd, est_elecrad);
                    break;
                }
                
                {
                    result = speed_ctrl.update(target, est_spd);
                    break;
                }
            }
            case CtrlType::TEACH:{
                real_t max_current = target;
                real_t spd = getSpeed();
                real_t abs_spd = ABS(spd);
                scexpr real_t deadzone = real_t(0.23);
                if(abs_spd < deadzone){
                    result = {0, 0}; 
                }else{
                    real_t expect_current = MIN(abs_spd * real_t(0.27), max_current);
                    result = {expect_current, SIGN_AS(real_t(PI / 2) *real_t(1.3), spd)};
                }
                break;
            }
        } 

        curr_ctrl.update(result);
        meta.curr = curr_ctrl.curr();
        run_leadangle = curr_ctrl.raddiff();
    }
}