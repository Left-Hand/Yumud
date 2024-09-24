#include "robots/foc/stepper/stepper.hpp"


void FOCStepper::active_task(){
    using Result = CtrlResult;
    scexpr auto ratio = real_t(0.5);

    if(ctrl_type == CtrlType::VECTOR){
        svpwm.setDuty(curr_ctrl.config.openloop_curr * ratio, odo.position2rad(target) + meta.radfix);
    }else{
        svpwm.setDuty(meta.curr * ratio, meta.elecrad + meta.raddiff + meta.radfix);
    }

    odo.update();

    meta.pos = odo.getPosition();
    meta.elecrad = odo.getElecRad();
    meta.spd = (speed_estmator.update(meta.pos) + meta.spd * 127) >> 7;

    {

        Result result;
        switch(ctrl_type){
            case CtrlType::CURRENT:{
                bool dir_correct = meta.spd * target >= 0; 
                real_t targ_curr = target;
                
                if(dir_correct){
                    result = {ABS(targ_curr), SIGN_AS(meta.get_max_raddiff(), targ_curr)};   
                }else{
                    result = {ABS(targ_curr) * MAX(real_t(1) - meta.spd * real_t(0.1), real_t(0)), SIGN_AS(real_t(PI/2), targ_curr)};
                }
                break;
            }
            case CtrlType::VECTOR:
                result = {meta.max_curr, 0};
                break;

            case CtrlType::POSITION:
                result = position_ctrl.update(target, meta.pos, meta.spd, meta.elecrad);
                break;
            case CtrlType::TRAPEZOID:
                result = trapezoid_ctrl.update(target, meta.pos, meta.spd, meta.elecrad);
                break;
    
            case CtrlType::SPEED:{
                scexpr real_t dead_zone = real_t(0.003);
                if((meta.pos >= meta.pos_limit.to - dead_zone and target > 0)
                     or (meta.pos <= meta.pos_limit.from + dead_zone and target < 0)){
                    result = position_ctrl.update((target > 0 ? meta.pos_limit.to : meta.pos_limit.from)
                            , meta.pos, meta.spd, meta.elecrad);
                    break;
                }
                
                {
                    result = speed_ctrl.update(target, meta.spd);
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
        meta.raddiff = curr_ctrl.raddiff();
    }
}