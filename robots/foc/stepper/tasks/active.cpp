#include "robots/foc/stepper/stepper.hpp"

using namespace yumud::foc;

void FOCStepper::active_task(){
    using Result = CtrlResult;
    scexpr auto ratio = real_t(0.5);

    if(ctrl_type == CtrlType::VECTOR){
        svpwm.setDuty(meta.openloop_curr * ratio, odo.position2rad(meta.targ_pos) + meta.radfix);
    }else{
        svpwm.setDuty(meta.curr * ratio, meta.elecrad + meta.raddiff + meta.radfix);
        // svpwm.setDuty(meta.curr * ratio, meta.elecrad + real_t(PI / 2  + meta.radfix);
    }

    odo.update();

    meta.pos = odo.getPosition();
    meta.elecrad = odo.getElecRad();
    meta.spd = speed_estmator.update(meta.pos);

    {
        auto is_pos_outrange = [&]() -> bool{
            scexpr real_t dead_zone = real_t(0.003);
            return ((meta.pos >= meta.pos_limit.to - dead_zone and meta.targ_spd > 0)
                or (meta.pos <= meta.pos_limit.from + dead_zone and meta.targ_spd < 0)) ;
        };
        
        Result result;
        switch(ctrl_type){
            case CtrlType::CURRENT:
                result = {ABS(meta.targ_curr), SIGN_AS(meta.get_max_raddiff(), meta.targ_curr)};   
                break;

            case CtrlType::VECTOR:
                result = {meta.max_curr, 0};
                break;

            case CtrlType::POSITION:
                meta.targ_est_spd = targ_spd_est.update(meta.targ_pos);
                result = position_ctrl.update(meta.pos_limit.clamp(meta.targ_pos), meta.pos, meta.spd);
                break;
    
            case CtrlType::SPEED:
                if(is_pos_outrange()){
                    result = position_ctrl.update((meta.targ_spd > 0 ? meta.pos_limit.to : meta.pos_limit.from)
                            , meta.pos, meta.spd);
                }else{
                    result = speed_ctrl.update(meta.targ_spd, meta.spd);
                }
                break;

            case CtrlType::TEACH:{
                real_t max_current = meta.targ_curr;
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

        // if(SIGN_DIFF(result.raddiff, meta.spd) and ABS(meta.spd) > 1){
        //     // result = {ABS(meta.max_curr) * MAX(real_t(1) - meta.spd * real_t(0.1), real_t(0)), SIGN_AS(real_t(PI/2), result.current)};
        //     result = {0, SIGN_AS(real_t(PI/2), result.raddiff)};
        // }
        
        curr_ctrl.update(result);
        meta.curr = curr_ctrl.curr();
        meta.raddiff = curr_ctrl.raddiff();
    }
}